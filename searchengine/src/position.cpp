#include <sstream>
#include <iostream>
#include <random>

#include "position.h"
#include "types.h"
#include "movegen.h"
#include "misc.h"

namespace harukashogi {


// put all the zobrist hash code related functions here to avoid confusion
namespace Zobrist {

    uint64_t boardKeys[NUM_SQUARES][NUM_PIECES];
    uint64_t handKeys[NUM_COLORS][NUM_UNPROMOTED_PIECE_TYPES][MAX_HAND_COUNT];
    uint64_t sideToMoveKey;

    void init() {
        // Create a 64-bit Mersenne Twister PRNG
        std::mt19937_64 rng(12345);
        
        // Generate random uint64_t values for board keys
        for (int sq = 0; sq < NUM_SQUARES; ++sq) {
            for (int piece = 0; piece < NUM_PIECES; ++piece) {
                boardKeys[sq][piece] = rng();
            }
        }
        
        // Generate random uint64_t values for hand keys
        for (int color = 0; color < NUM_COLORS; ++color) {
            for (int pieceType = 0; pieceType < NUM_UNPROMOTED_PIECE_TYPES; ++pieceType) {
                for (int count = 0; count < MAX_HAND_COUNT; ++count) {
                    handKeys[color][pieceType][count] = rng();
                }
            }
        }
        
        // Generate random uint64_t value for side to move
        sideToMoveKey = rng();
    }
}


// initializes the position
void Position::init() {
    Zobrist::init();
}


// initializes the position from a SFEN string
void Position::set(const std::string& sfenStr) {
    char token;
    size_t idx;
    bool promote = false;
    Color color;
    Square sq = SQ_91;
    std::istringstream ss(sfenStr);

    ss >> std::noskipws;

    // empty board and hands
    board.fill(NO_PIECE);
    hands.fill(0);
    pawnFiles.fill(false);
    gameStatus = NO_STATUS;
    winner = NO_COLOR;

    // 1. board pieces
    while ((ss >> token) && token != ' ') {
        if (isdigit(token))
            sq += (token - '0') * EAST;

        else if (token == '/')
            sq += 2 * SOUTH;

        else if (token == '+')
            promote = true;

        else if ((idx = PieceToChar.find(token)) != std::string::npos) {
            Piece p = promote ? promote_piece(Piece(idx)) : Piece(idx);
            add_piece(p, sq);
            // update the king location
            if (type_of(board[sq]) == KING)
                kingSq[color_of(board[sq])] = sq;
            // update the pawn file
            if (type_of(board[sq]) == PAWN)
                pawnFiles[color_of(board[sq]) * NUM_FILES + file_of(sq)] = true;
            promote = false;
            sq += EAST;
        }
    }

    // 2. side to move
    ss >> token;
    sideToMove = (token == 'b') ? BLACK : WHITE;
    ss >> token;

    // 3. hand pieces
    while ((ss >> token) && token != ' ') {
        color = isupper(token) ? BLACK : WHITE;
        if ((idx = PieceToChar.find(tolower(token))) != std::string::npos)
            add_hand_piece(color, type_of(Piece(idx)));
    }

    // 4. full move count
    ss >> std::skipws >> gamePly;
    gamePly--;

    // initialize the state info list
    si.clear();
    si.push_front(StateInfo());

    // compute the zobrist hash code
    compute_key();

    // initialize the repetition table and add the initial position
    repetitionTable = RepetitionTable();
    repetitionTable.add(si.front().key);
}


// returns the SFEN string representation of the position
std::string Position::sfen() const {
    std::stringstream ss;
    Square sq;
    int empty_count = 0;
    bool hands_empty = true;
    Color c = BLACK;

    // 1. board pieces
    for (Rank r = R_1; r < NUM_RANKS; ++r) {
        for (File f = F_9; f < NUM_FILES; --f) {
            sq = make_square(f, r);

            if (board[sq] == NO_PIECE) 
                empty_count++;

            else {
                if (empty_count > 0) {
                    ss << empty_count;
                    empty_count = 0;
                }
                
                if (is_promoted(board[sq])) {
                    ss << '+';
                    ss << PieceToChar[unpromote_piece(board[sq])];
                }
                else
                    ss << PieceToChar[board[sq]];
            }
        }

        if (empty_count > 0) {
            ss << empty_count;
            empty_count = 0;
        }
        if (r != R_9)
            ss << '/';
    }

    // 2. side to move
    ss << ' ';
    ss << (sideToMove == BLACK ? 'b' : 'w');
    ss << ' ';

    // 3. hand pieces
    for (int i = 0; i < NUM_COLORS; ++i) {
        for (PieceType pt = GOLD; pt < NUM_UNPROMOTED_PIECE_TYPES; ++pt) {
            for (int count = 0; count < hands[c * NUM_UNPROMOTED_PIECE_TYPES + pt]; ++count) {
                ss << PieceToChar[make_piece(c, pt)];
                hands_empty = false;
            }
        }

        c = ~c;
    }
    if (hands_empty)
        ss << '-';

    // 4. full move count
    ss << ' ' << gamePly + 1;

    return ss.str();
}


// makes the given move.
// the move is assumed to be legal.
void Position::make_move(Move m) {
    uint8_t count;

    // add a new state info to the list
    // copy the current state info to the new one
    si.push_front(StateInfo(si.front()));
    si.front().checkStatus.fill(CHECK_UNRESOLVED);
    si.front().capturedPT = NO_PIECE_TYPE;

    // move is not a drop
    if (!m.is_drop()) {
        // update the zobrist key by removing the piece from the from square
        si.front().key ^= Zobrist::boardKeys[m.from()][board[m.from()]];

        Piece p = board[m.from()];

        // capture
        if (board[m.to()] != NO_PIECE) {
            PieceType capturedPT = type_of(board[m.to()]);
            // unpromote the piece before adding to hand
            count = hands[sideToMove * NUM_UNPROMOTED_PIECE_TYPES + unpromoted_type(capturedPT)];
            add_hand_piece(sideToMove, unpromoted_type(capturedPT));
            // handle pawn files if a pawn is captured
            if (capturedPT == PAWN)
                pawnFiles[~sideToMove * NUM_FILES + file_of(m.to())] = false;

            // update the zobrist key by removing the captured piece from the to square
            // and adding the captured piece to the hand
            si.front().key ^= Zobrist::boardKeys[m.to()][board[m.to()]];
            si.front().key ^= Zobrist::handKeys[sideToMove][unpromoted_type(capturedPT)][count];

            // update the captured piece type in the state info
            si.front().capturedPT = capturedPT;
        }

        // promote the piece if it's a promotion
        // (remove the unpromoted piece and add the promoted piece)
        if (m.is_promotion()) {
            remove_piece(m.from());
            add_piece(promote_piece(p), m.to());
            // if a pawn is promoted, update the pawn file
            // pawns can be dropped to tha same file of the promoted pawn
            if (type_of(board[m.to()]) == P_PAWN) {
                pawnFiles[sideToMove * NUM_FILES + file_of(m.from())] = false;
            }
        }
        // otherwise just move the piece
        else {
            move_piece(m.from(), m.to());
        }

        // update the zobrist key by adding the piece after the move to the to square
        si.front().key ^= Zobrist::boardKeys[m.to()][board[m.to()]];

        // update king square
        if (type_of(p) == KING)
            kingSq[sideToMove] = m.to();
    }
    // drop
    else {
        remove_hand_piece(sideToMove, m.dropped());
        count = hands[sideToMove * NUM_UNPROMOTED_PIECE_TYPES + m.dropped()];
        add_piece(make_piece(sideToMove, m.dropped()), m.to());
        // handle pawn files if a pawn is dropped
        if (m.dropped() == PAWN)
            pawnFiles[sideToMove * NUM_FILES + file_of(m.to())] = true;

        // update the zobrist key
        si.front().key ^= Zobrist::handKeys[sideToMove][m.dropped()][count];
        si.front().key ^= Zobrist::boardKeys[m.to()][board[m.to()]];
    }

    // update side to move and game ply
    si.front().checkStatus.fill(CHECK_UNRESOLVED);
    gameStatus = NO_STATUS;
    sideToMove = ~sideToMove;
    gamePly++;

    // update the zobrist key by toggling the side to move
    si.front().key ^= Zobrist::sideToMoveKey;

    // compute_key();

    // update the repetition table
    repetitionTable.add(si.front().key);
}


// undoes the given move.
// the move is assumed to be legal.
void Position::unmake_move(Move m) {
    uint8_t count;

    // update the repetition table before removing the state info
    repetitionTable.remove(si.front().key);

    // update side to move first makes logic more intuitive
    // this way sideToMove is from before the move was made
    sideToMove = ~sideToMove;
    gamePly--;

    // update game status
    gameStatus = NO_STATUS;
    winner = NO_COLOR;

    // move is not a drop
    if (!m.is_drop()) {

        Piece p = board[m.to()];

        // unpromote the piece if it's a promotion
        // (remove the promoted piece and add the unpromoted piece)
        if (m.is_promotion()) {
            remove_piece(m.to());
            add_piece(unpromote_piece(p), m.from());
            // if the promoted piece was a pawn, update the pawn file
            if (type_of(board[m.from()]) == PAWN) {
                pawnFiles[sideToMove * NUM_FILES + file_of(m.from())] = true;
            }
        }
        // otherwise just move the piece
        else {
            move_piece(m.to(), m.from());
        }

        // capture
        // (remove the captured piece from the hand and add it to the board)
        if (si.front().capturedPT != NO_PIECE_TYPE) {
            PieceType capturedPT = si.front().capturedPT;
            // piece was promoted, so unpromote it before removing from hand
            remove_hand_piece(sideToMove, unpromoted_type(capturedPT));
            count = hands[sideToMove * NUM_UNPROMOTED_PIECE_TYPES + unpromoted_type(capturedPT)];
            add_piece(make_piece(~sideToMove, capturedPT), m.to());
            // handle pawn files if a pawn was captured and is now on the board
            if (capturedPT == PAWN)
                pawnFiles[~sideToMove * NUM_FILES + file_of(m.to())] = true;
        }

        // update king square
        if (type_of(board[m.from()]) == KING)
            kingSq[sideToMove] = m.from();
    }

    // move is a drop
    else {
        count = hands[sideToMove * NUM_UNPROMOTED_PIECE_TYPES + m.dropped()];
        add_hand_piece(sideToMove, m.dropped());

        remove_piece(m.to());
        // handle pawn files if a pawn was dropped and is now not on the board
        if (m.dropped() == PAWN)
            pawnFiles[sideToMove * NUM_FILES + file_of(m.to())] = false;
    }

    // remove the current state info from the list
    si.pop_front();
}


bool Position::is_in_check(Color color) {
    CheckStatus& checkStatus = si.front().checkStatus[color];
    // CheckStatus checkStatus = CHECK_UNRESOLVED;
    if (checkStatus == CHECK_UNRESOLVED) {
        checkStatus = is_attacked(*this, kingSq[color], ~color) ? CHECK : NOT_CHECK;
    }
    return checkStatus == CHECK;
}


bool Position::is_legal(Move m) {
    if (m.is_null())
        return false;

    // TODO: this is unoptimized, needs to be optimized
    make_move(m);
    bool is_legal = !is_in_check(~sideToMove);

    // pawn drop can't checkmate
    // TODO: this works but is very inefficient
    // generate pawn move and check for checkmate
    if (is_legal && m.is_drop() && m.dropped() == PAWN) {
        DirectionStruct pawnAttack = (sideToMove == BLACK) ? SOUTH : NORTH;
        // if the pawn drop eats the king, check if there are any legal moves
        // for the opponent. If not it's checkmate.
        if ((m.to() + pawnAttack) == kingSq[sideToMove]) {
            Move moveList[MAX_MOVES];
            legacy_generate_moves(*this, moveList);
            if (moveList[0].is_null())
                is_legal = false;   
        }
    }

    unmake_move(m);

    return is_legal;
}


bool Position::is_capture(Move m) const {
    return board[m.to()] != NO_PIECE;
}


bool Position::is_checkmate() {
    Color color = sideToMove;
    // only go forward if the king is in check
    if (is_in_check(color)) {
        // first check if the king has any legal moves
        Move moveList[MAX_MOVES];
        Move* end = piece_moves(*this, moveList, kingSq[color]);

        // if the king has no legal move, generate all moves
        if (end == moveList) {
            end = legacy_generate_moves(*this, moveList);
            if (end == moveList) {
                gameStatus = GAME_OVER;
                winner = ~color;
                return true;
            }
        }
    }

    gameStatus = IN_PROGRESS;
    return false;
}


bool Position::is_game_over() {
    if (gameStatus == NO_STATUS) {
        if (repetitionTable.reached_repetitions(si.front().key, si)) {
            gameStatus = GAME_OVER;
            winner = NO_COLOR;
        }
        else
            is_checkmate();
    }
        

    if (gameStatus == GAME_OVER)
        return true;
    else
        return false;
}


Color Position::get_winner() const {
    return winner;
}


void Position::add_piece(Piece p, Square sq) {
    // update the board
    board[sq] = p;

    // update the bitboards
    allPiecesBB[color_of(p)] |= square_bb(sq);
    // king is excluded from dirPieces as the moves are handled separately
    if (type_of(p) != KING) {
        for (int i = 0; i < 8 && PTDirections[p-1][i] != NULL_DIR; ++i) {
            dirPieces[color_of(p)][PTDirections[p-1][i]] |= square_bb(sq);
        }
    }

    int sl_idx = sliding_type_index(type_of(p));
    if (sl_idx >= 0) {
        slPieces[color_of(p)][sl_idx] |= square_bb(sq);
    }
}


void Position::remove_piece(Square sq) {
    Piece p = board[sq];

    // update the board
    board[sq] = NO_PIECE;

    // update the bitboards
    allPiecesBB[color_of(p)] ^= square_bb(sq);
    // king is excluded from dirPieces as the moves are handled separately
    if (type_of(p) != KING) {
        for (int i = 0; i < 8 && PTDirections[p-1][i] != NULL_DIR; ++i) {
            dirPieces[color_of(p)][PTDirections[p-1][i]] ^= square_bb(sq);
        }
    }

    int sl_idx = sliding_type_index(type_of(p));
    if (sl_idx >= 0) {
        slPieces[color_of(p)][sl_idx] ^= square_bb(sq);
    }
}


void Position::move_piece(Square from, Square to) {
    Piece p = board[from];

    // update the board
    board[from] = NO_PIECE;
    board[to] = p;

    // update the bitboards
    allPiecesBB[color_of(p)] ^= square_bb(from) | square_bb(to);
    // king is excluded from dirPieces as the moves are handled separately
    if (type_of(p) != KING) {
        for (int i = 0; i < 8 && PTDirections[p-1][i] != NULL_DIR; ++i) {
            dirPieces[color_of(p)][PTDirections[p-1][i]] ^= square_bb(from) | square_bb(to);
        }
    }

    int sl_idx = sliding_type_index(type_of(p));
    if (sl_idx >= 0) {
        slPieces[color_of(p)][sl_idx] ^= square_bb(from) | square_bb(to);
    }
}


void Position::add_hand_piece(Color color, PieceType pt) {
    hands[color * NUM_UNPROMOTED_PIECE_TYPES + pt]++;
}


void Position::remove_hand_piece(Color color, PieceType pt) {
    hands[color * NUM_UNPROMOTED_PIECE_TYPES + pt]--;
}


// computes the zobrist hash code of the position
// this is only called when setting the position
// otherwise it's updated incrementally in make_move and unmake_move
void Position::compute_key() {
    uint64_t key = 0;

    // board pieces
    for (Square sq = SQ_11; sq < NUM_SQUARES; ++sq) {
        if (board[sq] != NO_PIECE) {
            key ^= Zobrist::boardKeys[sq][board[sq]];
        }
    }

    // hand pieces
    for (Color color = BLACK; color < NUM_COLORS; ++color) {
        for (PieceType pt = GOLD; pt < NUM_UNPROMOTED_PIECE_TYPES; ++pt) {
            for (uint8_t count = 0; count < hands[color * NUM_UNPROMOTED_PIECE_TYPES + pt]; ++count)
                key ^= Zobrist::handKeys[color][pt][count];
        }
    }

    // side to move
    if (sideToMove == BLACK)
        key ^= Zobrist::sideToMoveKey;

    si.front().key = key;
}


bool RepetitionTable::reached_repetitions(
        uint64_t key,
        std::forward_list<StateInfo>& si,
        uint8_t nRepetitions
    ) {
    // if the count is less than the draw repetition limit, return the count
    // even if it's wrong, it doesn't matter
    if (table[index(key)] < nRepetitions) {
        return false;
    }

    else {
        countsNeeded++;
        // search backwards in the key history to count the repetitions
        // this is more efficient than searching forwards in realistic scenarios
        int wrongHits = 0;
        int count = 0;
        for (StateInfo& st : si) {
            if (index(st.key) == index(key)) {
                if (st.key == key) {
                    count++;
                    if (count >= nRepetitions) {
                        repetitions++;
                        return true;
                    }
                        
                }
                else
                    wrongHits++;
                // if the remaining hits are less than the repetitions limit, exit early and
                // return false
                if (table[index(st.key)] - wrongHits < nRepetitions)
                    return false;
            }
        }

        return false;
    }
}

} // namespace harukashogi