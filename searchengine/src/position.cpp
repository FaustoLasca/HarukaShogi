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
    checkStatus.fill(CHECK_UNRESOLVED);
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
            board[sq] = promote ? promote_piece(Piece(idx)) : Piece(idx);
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
            hands[color * NUM_UNPROMOTED_PIECE_TYPES + type_of(Piece(idx))]++;
    }

    // 4. full move count
    ss >> std::skipws >> gamePly;
    gamePly--;

    // compute the zobrist hash code
    compute_key();
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

    // move is not a drop
    if (m.from != NO_SQUARE) {
        // update the zobrist key by removing the piece from the from square
        key ^= Zobrist::boardKeys[m.from][board[m.from]];

        // capture
        if (m.type_involved != NO_PIECE_TYPE) {
            // unpromote the piece before adding to hand
            count = hands[sideToMove * NUM_UNPROMOTED_PIECE_TYPES + unpromoted_type(m.type_involved)];
            hands[sideToMove * NUM_UNPROMOTED_PIECE_TYPES + unpromoted_type(m.type_involved)]++;
            // handle pawn files if a pawn is captured
            if (m.type_involved == PAWN)
                pawnFiles[~sideToMove * NUM_FILES + file_of(m.to)] = false;

            // update the zobrist key by removing the captured piece from the to square
            // and adding the captured piece to the hand
            key ^= Zobrist::boardKeys[m.to][board[m.to]];
            key ^= Zobrist::handKeys[sideToMove][unpromoted_type(m.type_involved)][count];
        }

        // update the board pieces
        board[m.to] = board[m.from];
        board[m.from] = NO_PIECE;
            
        // promote the piece if it's a promotion
        if (m.promotion) {
            board[m.to] = promote_piece(board[m.to]);
            // if a pawn is promoted, update the pawn file
            // pawns can be dropped to tha same file of the promoted pawn
            if (type_of(board[m.to]) == P_PAWN) {
                pawnFiles[sideToMove * NUM_FILES + file_of(m.from)] = false;
            }
        }

        // update the zobrist key by adding the piece after the move to the to square
        key ^= Zobrist::boardKeys[m.to][board[m.to]];

        // update king square
        if (type_of(board[m.to]) == KING)
            kingSq[sideToMove] = m.to;
    }
    // drop
    else {
        hands[sideToMove * NUM_UNPROMOTED_PIECE_TYPES + m.type_involved]--;
        count = hands[sideToMove * NUM_UNPROMOTED_PIECE_TYPES + m.type_involved];
        board[m.to] = make_piece(sideToMove, m.type_involved);
        // handle pawn files if a pawn is dropped
        if (m.type_involved == PAWN)
            pawnFiles[sideToMove * NUM_FILES + file_of(m.to)] = true;

        // update the zobrist key
        key ^= Zobrist::handKeys[sideToMove][m.type_involved][count];
        key ^= Zobrist::boardKeys[m.to][board[m.to]];
    }

    // update side to move and game ply
    checkStatus.fill(CHECK_UNRESOLVED);
    gameStatus = NO_STATUS;
    sideToMove = ~sideToMove;
    gamePly++;

    // update the zobrist key by toggling the side to move
    key ^= Zobrist::sideToMoveKey;

    // compute_key();
}


// undoes the given move.
// the move is assumed to be legal.
void Position::unmake_move(Move m) {
    uint8_t count;

    // update side to move first makes logic more intuitive
    // this way sideToMove is from before the move was made
    sideToMove = ~sideToMove;
    gamePly--;

    // update game status
    gameStatus = IN_PROGRESS;
    checkStatus.fill(CHECK_UNRESOLVED);
    winner = NO_COLOR;

    // move is not a drop
    if (m.from != NO_SQUARE) {
        // remove the piece from the zobrist key by removing the piece from the to square
        key ^= Zobrist::boardKeys[m.to][board[m.to]];

        // update the board
        board[m.from] = board[m.to];

        // capture
        if (m.type_involved != NO_PIECE_TYPE) {
            // piece was promoted, so unpromote it before removing from hand
            hands[sideToMove * NUM_UNPROMOTED_PIECE_TYPES + unpromoted_type(m.type_involved)]--;
            count = hands[sideToMove * NUM_UNPROMOTED_PIECE_TYPES + unpromoted_type(m.type_involved)];
            board[m.to] = make_piece(~sideToMove, m.type_involved);
            // handle pawn files if a pawn was captured and is now on the board
            if (m.type_involved == PAWN)
                pawnFiles[~sideToMove * NUM_FILES + file_of(m.to)] = true;

            // update the zobrist key by adding the captured piece to the to square
            // and removing the captured piece from the hand
            key ^= Zobrist::boardKeys[m.to][board[m.to]];
            key ^= Zobrist::handKeys[sideToMove][unpromoted_type(m.type_involved)][count];
        }
        else
            board[m.to] = NO_PIECE;
            
        // unpromote the piece if it's a promotion
        if (m.promotion) {
            board[m.from] = unpromote_piece(board[m.from]);
            // if the promoted piece was a pawn, update the pawn file
            if (type_of(board[m.from]) == PAWN) {
                pawnFiles[sideToMove * NUM_FILES + file_of(m.from)] = true;
            }
        }

        // update the zobrist key by adding the piece to the from square
        key ^= Zobrist::boardKeys[m.from][board[m.from]];

        // update king square
        if (type_of(board[m.from]) == KING)
            kingSq[sideToMove] = m.from;
    }

    // move is a drop
    else {
        count = hands[sideToMove * NUM_UNPROMOTED_PIECE_TYPES + m.type_involved];
        hands[sideToMove * NUM_UNPROMOTED_PIECE_TYPES + m.type_involved]++;

        // update the zobrist key by adding the piece to the hand
        // and removing the piece from the to square
        key ^= Zobrist::handKeys[sideToMove][m.type_involved][count];
        key ^= Zobrist::boardKeys[m.to][board[m.to]];

        board[m.to] = NO_PIECE;
        // handle pawn files if a pawn was dropped and is now not on the board
        if (m.type_involved == PAWN)
            pawnFiles[sideToMove * NUM_FILES + file_of(m.to)] = false;
    }

    // update the zobrist key by toggling the side to move
    key ^= Zobrist::sideToMoveKey;

    // compute_key();
}


bool Position::is_in_check(Color color) {
    if (checkStatus[color] == CHECK_UNRESOLVED) {
        checkStatus[color] = is_attacked(*this, kingSq[color], ~color) ? CHECK : NOT_CHECK;
    }
    return checkStatus[color] == CHECK;
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
    if (is_legal && m.is_drop() && m.type_involved == PAWN) {
        Direction pawnAttack = (sideToMove == BLACK) ? SOUTH : NORTH;
        // if the pawn drop eats the king, check if there are any legal moves
        // for the opponent. If not it's checkmate.
        if ((m.to + pawnAttack) == kingSq[sideToMove]) {
            Move moveList[MAX_MOVES];
            generate_moves(*this, moveList);
            if (moveList[0].is_null())
                is_legal = false;   
        }
    }

    unmake_move(m);

    return is_legal;
}


bool Position::is_checkmate() {
    Color color = sideToMove;
    // only go forward if the king is in check
    if (is_in_check(color)) {
        // first check if the king has any legal moves
        Move moveList[MAX_MOVES];
        piece_moves(*this, moveList, kingSq[color]);

        // if the king has no legal move, generate all moves
        if (moveList[0].is_null()) {
            generate_moves(*this, moveList);
            if (moveList[0].is_null()) {
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
    if (gameStatus == NO_STATUS)
        is_checkmate();

    if (gameStatus == GAME_OVER)
        return true;

    // no check for stalemate atm
    return false;
}


Color Position::get_winner() const {
    return winner;
}


// computes the zobrist hash code of the position
// this is only called when setting the position
// otherwise it's updated incrementally in make_move and unmake_move
void Position::compute_key() {
    key = 0;

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
}


} // namespace harukashogi