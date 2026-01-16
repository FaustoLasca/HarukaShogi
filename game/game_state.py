from collections import defaultdict
from game.misc import Player, PieceType, Piece, PROMOTABLE, Change, STARTING_PIECE_LIST
from typing import List

class GameState:
    def __init__(self):
        self.board = [None] * 81
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

    def move(self, move: List[Change]):
        for change in move:
            if change.from_pos == None:
                self.hand[change.player][change.piece_type] -= 1
            else:
                c, r = change.from_pos
                self.board[c*9 + r] = None
                self.piece_list[change.player][(change.piece_type, change.promoted)].remove(change.from_pos)
            
            if change.to_pos == None:
                self.hand[change.player][change.piece_type] += 1
            else:
                c, r = change.to_pos
                self.board[c*9 + r] = Piece(change.player, change.piece_type, change.promoted)
                self.piece_list[change.player][(change.piece_type, change.promoted)].add(change.to_pos)

    
    def unmove(self, move):
        pass

