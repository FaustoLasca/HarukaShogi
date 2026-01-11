from collections import defaultdict
from game.misc import Player, PieceType, Piece, PROMOTABLE, Change
from collections import List

class GameState:
    def __init__(self):
        self.board = [None] * 81
        self.hand = {
            Player.BLACK: defaultdict[PieceType, int](int),
            Player.WHITE: defaultdict[PieceType, int](int)
        }
        # to make move generation faster
        self.piece_list = {
            Player.BLACK: defaultdict[PieceType, set](set),
            Player.WHITE: defaultdict[PieceType, set](set)
        }
        self.current_player = Player.BLACK


    def generate_moves(self):
        pass


    def move(self, move: List[Change]):
        for change in move:
            if change.from_pos == None:
                self.hand[change.player][change.piece_type] -= 1
            else:
                r, c = change.from_pos
                self.board[r*9 + c] = None
                self.piece_list[change.player][change.piece_type].remove(change.from_pos)
            
            if change.to_pos == None:
                self.hand[change.player][change.piece_type] += 1
            else:
                r, c = change.to_pos
                self.board[r*9 + c] = Piece(change.player, change.piece_type, change.promoted)
                self.piece_list[change.player][change.piece_type].add(change.to_pos)

    
    def unmove(self, move):
        pass

