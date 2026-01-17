from collections import defaultdict
from typing import List

from game.move_rules import FIXED_MOVES_DICT, SLIDING_MOVES_DICT
from game.misc import Player, PieceType, Piece, PROMOTABLE, Change, STARTING_PIECE_LIST

class GameState:
    def __init__(self):
        self.board : List[Piece | None] = [None] * 81
        self.hand = {
            Player.BLACK: defaultdict[PieceType, int](int),
            Player.WHITE: defaultdict[PieceType, int](int),
        }
        # to make move generation faster
        self.piece_list = {
            Player.BLACK: defaultdict[tuple[PieceType, bool], set[tuple[int, int]]](set),
            Player.WHITE: defaultdict[tuple[PieceType, bool], set[tuple[int, int]]](set),
        }
        self.current_player = Player.BLACK
        self.initialize_board()

    def initialize_board(self):
        for piece, cells in STARTING_PIECE_LIST.items():
            self.piece_list[piece.player][(piece.type, piece.promoted)] = cells
            for cell in cells:
                col, row = cell
                self.board[col*9 + row] = piece

    def generate_moves(self):
        pass

    def generate_pseudo_legal_moves(self) -> List[List[Change]]:
        moves = []

        # generate moves for pieces on the board
        for piece_type, promoted in self.piece_list[self.current_player].keys():
            for from_col, from_row in self.piece_list[self.current_player][(piece_type, promoted)]:
                piece = Piece(self.current_player, piece_type, promoted)
                # check fixed moves first
                for delta_col, delta_row in FIXED_MOVES_DICT[piece]:
                    to_col = from_col + delta_col
                    to_row = from_row + delta_row
                    # check if the move is on the board
                    if 0 <= to_col < 9 and 0 <= to_row < 9:
                        forced_promotion = not promoted and (
                                (piece_type == PieceType.PAWN and self.current_player == Player.BLACK and to_row == 0) or 
                                (piece_type == PieceType.PAWN and self.current_player == Player.WHITE and to_row == 8) or
                                (piece_type == PieceType.KNIGHT and self.current_player == Player.BLACK and to_row <= 1) or
                                (piece_type == PieceType.KNIGHT and self.current_player == Player.WHITE and to_row >= 7)
                            )
                        # check if the square is empty
                        if self.board[to_col*9 + to_row] is None:
                            if not forced_promotion:
                                moves.append(
                                    [
                                        Change(self.current_player, piece_type, promoted, promoted, (from_col, from_row), (to_col, to_row))
                                    ]
                                )
                            # check if the piece can be promoted
                            if piece_type in PROMOTABLE and not promoted:
                                if (self.current_player == Player.BLACK and (to_row < 3 or from_row < 3)) or (self.current_player == Player.WHITE and (to_row >= 6 or from_row >= 6)):
                                    moves.append(
                                        [
                                            Change(self.current_player, piece_type, promoted, True, (from_col, from_row), (to_col, to_row))
                                        ]
                                    )
                        # check if the square is occupied by the opponent
                        elif self.board[to_col*9 + to_row].player != self.current_player:
                            captured_piece = self.board[to_col*9 + to_row]
                            if not forced_promotion:
                                moves.append(
                                    [
                                        Change(captured_piece.player, captured_piece.type, captured_piece.promoted, False, (to_col, to_row), None),
                                        Change(self.current_player, piece_type, promoted, promoted, (from_col, from_row), (to_col, to_row))
                                    ]
                                )
                            if piece_type in PROMOTABLE and not promoted:
                                if (self.current_player == Player.BLACK and (to_row < 3 or from_row < 3)) or (self.current_player == Player.WHITE and (to_row >= 6 or from_row >= 6)):
                                    moves.append(
                                        [
                                            Change(captured_piece.player, captured_piece.type, captured_piece.promoted, False, (to_col, to_row), None),
                                            Change(self.current_player, piece_type, promoted, True, (from_col, from_row), (to_col, to_row))
                                        ]
                                    )
                # check sliding moves
                if piece in SLIDING_MOVES_DICT:
                    for delta_col, delta_row in SLIDING_MOVES_DICT[piece]:
                        to_col = from_col
                        to_row = from_row
                        while 0 <= to_col + delta_col < 9 and 0 <= to_row + delta_row < 9:
                            to_col += delta_col
                            to_row += delta_row
                            forced_promotion = not promoted and (
                                    (piece_type == PieceType.LANCE and self.current_player == Player.BLACK and to_row == 0) or
                                    (piece_type == PieceType.LANCE and self.current_player == Player.WHITE and to_row == 8)
                                )
                            # if the square is empty, add the move and continue
                            if self.board[to_col*9 + to_row] is None:
                                if not forced_promotion:
                                    moves.append(
                                        [
                                            Change(self.current_player, piece_type, promoted, promoted, (from_col, from_row), (to_col, to_row))
                                        ]
                                    )
                                # check if the piece can be promoted
                                if not promoted:
                                    if (self.current_player == Player.BLACK and (to_row < 3 or from_row < 3)) or (self.current_player == Player.WHITE and (to_row >= 6 or from_row >= 6)):
                                        moves.append(
                                            [
                                                Change(self.current_player, piece_type, promoted, True, (from_col, from_row), (to_col, to_row))
                                            ]
                                        )
                            # if the square is occupied by the opponent add the move and break
                            elif self.board[to_col*9 + to_row].player != self.current_player:
                                captured_piece = self.board[to_col*9 + to_row]
                                if not forced_promotion:
                                    moves.append(
                                        [
                                            Change(captured_piece.player, captured_piece.type, captured_piece.promoted, False, (to_col, to_row), None),
                                            Change(self.current_player, piece_type, promoted, promoted, (from_col, from_row), (to_col, to_row))
                                        ]
                                    )
                                # check if the piece can be promoted
                                if not promoted:
                                    if (self.current_player == Player.BLACK and (to_row < 3 or from_row < 3)) or (self.current_player == Player.WHITE and (to_row >= 6 or from_row >= 6)):
                                        moves.append(
                                            [
                                                Change(captured_piece.player, captured_piece.type, captured_piece.promoted, False, (to_col, to_row), None),
                                                Change(self.current_player, piece_type, promoted, True, (from_col, from_row), (to_col, to_row))
                                            ]
                                        )
                                break
                            # if the square is occupied by the same player break
                            else:
                                break

        # generate drop moves
        for piece_type, count in self.hand[self.current_player].items():
            if count > 0:
                min_row = 0
                max_row = 8
                # can't drop pawns, lances or knights on the last rank/ranks
                if piece_type in {PieceType.PAWN, PieceType.LANCE}:
                    if self.current_player == Player.BLACK:
                        min_row = 1
                    else:
                        max_row = 7
                elif piece_type == PieceType.KNIGHT:
                    if self.current_player == Player.BLACK:
                        min_row = 2
                    else:
                        max_row = 6
                # can't drop pawns on cols with other pawns
                cols = set(range(9))
                if piece_type == PieceType.PAWN:
                    for col, _ in self.piece_list[self.current_player][(PieceType.PAWN, False)]:
                        if col in cols:
                            cols.remove(col)
                # loop through the remaining rows and cols
                for row in range(min_row, max_row + 1):
                    for col in cols:
                        if self.board[col*9 + row] is None:
                            move =[Change(self.current_player, piece_type, False, False, None, (col, row))]
                            if piece_type == PieceType.PAWN:
                                # TODO: check if pawn checkmate
                                moves.append(move)
                            else:
                                moves.append(move)
        return moves


    def is_square_attacked(self, player: Player, square: tuple[int, int]) -> bool:
        pass

    def move(self, move: List[Change]):
        for change in move:
            # remove piece from the old position
            if change.from_pos == None:
                self.hand[change.player][change.piece_type] -= 1
            else:
                c, r = change.from_pos
                self.board[c*9 + r] = None
                self.piece_list[change.player][(change.piece_type, change.from_promoted)].remove(change.from_pos)
            # add piece to the new position
            if change.to_pos == None:
                # the piece goes in the opponent's hand
                self.hand[1 - change.player][change.piece_type] += 1
            else:
                c, r = change.to_pos
                self.board[c*9 + r] = Piece(change.player, change.piece_type, change.to_promoted)
                self.piece_list[change.player][(change.piece_type, change.to_promoted)].add(change.to_pos)
        # switch player
        self.current_player = 1 - self.current_player

    
    def unmove(self, move: List[Change]):
        for change in reversed(move):
            # remove piece from the new position
            if change.to_pos == None:
                # the piece went in the opponent's hand
                self.hand[1 - change.player][change.piece_type] -= 1
            else:
                c, r = change.to_pos
                self.board[c*9 + r] = None
                self.piece_list[change.player][(change.piece_type, change.to_promoted)].remove(change.to_pos)
            # add piece to the old position
            if change.from_pos == None:
                self.hand[change.player][change.piece_type] += 1
            else:
                c, r = change.from_pos
                self.board[c*9 + r] = Piece(change.player, change.piece_type, change.from_promoted)
                self.piece_list[change.player][(change.piece_type, change.from_promoted)].add(change.from_pos)
        # switch player
        self.current_player = 1 - self.current_player