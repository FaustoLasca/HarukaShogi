#include <sstream>
#include <iostream>

#include "position.h"
#include "types.h"
#include "movegen.h"
#include "misc.h"

namespace harukashogi {


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
    // move is not a drop
    if (m.from != NO_SQUARE) {
        board[m.to] = board[m.from];
        board[m.from] = NO_PIECE;

        // capture
        if (m.type_involved != NO_PIECE_TYPE) {
            // unpromote the piece before adding to hand
            hands[sideToMove * NUM_UNPROMOTED_PIECE_TYPES + unpromoted_type(m.type_involved)]++;
            // handle pawn files if a pawn is captured
            if (m.type_involved == PAWN)
                pawnFiles[~sideToMove * NUM_FILES + file_of(m.to)] = false;
        }
            

        if (m.promotion)
            board[m.to] = promote_piece(board[m.to]);

        // update king square
        if (type_of(board[m.to]) == KING)
            kingSq[sideToMove] = m.to;
    }
    // drop
    else {
        hands[sideToMove * NUM_UNPROMOTED_PIECE_TYPES + m.type_involved]--;
        board[m.to] = make_piece(sideToMove, m.type_involved);
        // handle pawn files if a pawn is dropped
        if (m.type_involved == PAWN)
            pawnFiles[sideToMove * NUM_FILES + file_of(m.to)] = true;
    }

    // update side to move and game ply
    sideToMove = ~sideToMove;
    gamePly++;
}


// undoes the given move.
// the move is assumed to be legal.
void Position::undo_move(Move m) {
    // update side to move first makes logic more intuitive
    // this way sideToMove is from before the move was made
    sideToMove = ~sideToMove;
    gamePly--;

    // move is not a drop
    if (m.from != NO_SQUARE) {
        board[m.from] = board[m.to];

        // capture
        if (m.type_involved != NO_PIECE_TYPE) {
            // piece was promoted, so unpromote it before removing from hand
            hands[sideToMove * NUM_UNPROMOTED_PIECE_TYPES + unpromoted_type(m.type_involved)]--;
            board[m.to] = make_piece(~sideToMove, m.type_involved);
            // handle pawn files if a pawn was captured and is now on the board
            if (m.type_involved == PAWN)
                pawnFiles[~sideToMove * NUM_FILES + file_of(m.to)] = true;
        }
        else
            board[m.to] = NO_PIECE;
            
        if (m.promotion)
            board[m.from] = unpromote_piece(board[m.from]);

        // update king square
        if (type_of(board[m.from]) == KING)
            kingSq[sideToMove] = m.from;
    }

    // move is a drop
    else {
        hands[sideToMove * NUM_UNPROMOTED_PIECE_TYPES + m.type_involved]++;
        board[m.to] = NO_PIECE;
        // handle pawn files if a pawn was dropped and is now not on the board
        if (m.type_involved == PAWN)
            pawnFiles[sideToMove * NUM_FILES + file_of(m.to)] = false;
    }
}


bool Position::is_in_check(Color color) const {
    return is_attacked(*this, kingSq[color], ~color);
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

    undo_move(m);

    return is_legal;
}


} // namespace harukashogi