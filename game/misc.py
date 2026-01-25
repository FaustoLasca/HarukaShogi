from enum import IntEnum, auto
from dataclasses import dataclass
from typing import List


class Player(IntEnum):
    BLACK = 0
    WHITE = 1

class PieceType(IntEnum):
    NONE = 0
    KING = auto()       
    GOLD = auto()
    SILVER = auto()
    KNIGHT = auto()
    LANCE = auto()
    BISHOP = auto()
    ROOK = auto()
    PAWN = auto()

@dataclass(frozen=True)
class Piece:
    player: Player
    type: PieceType
    promoted: bool = False

PROMOTABLE = {
    PieceType.SILVER,
    PieceType.KNIGHT,
    PieceType.LANCE,
    PieceType.BISHOP,
    PieceType.ROOK,
    PieceType.PAWN
}

@dataclass
class Change:
    player: Player
    piece_type: PieceType
    from_promoted: bool
    to_promoted: bool
    from_pos: tuple[int, int]
    to_pos: tuple[int, int]


CHAR_TO_PIECETYPE = {
    'K': PieceType.KING,
    'G': PieceType.GOLD,
    'S': PieceType.SILVER,
    'N': PieceType.KNIGHT,
    'L': PieceType.LANCE,
    'B': PieceType.BISHOP,
    'R': PieceType.ROOK,
    'P': PieceType.PAWN
}

def string_to_move(move_str: str, state) -> List[Change]:
    move = []

    # the player of the main change is the current player
    player = state.current_player
    
    # translate the from positions and check the piece type and promoted status
    if move_str[1] == '*':
        from_pos = None
        piece_type = CHAR_TO_PIECETYPE[move_str[0]]
        from_promoted = False
    else:
        from_pos = (int(move_str[0]) - 1, ord(move_str[1]) - ord('a'))
        from_idx = from_pos[0] * 9 + from_pos[1]
        piece_type = state.board[from_idx].type
        from_promoted = state.board[from_idx].promoted
    
    # translate the to position from the move string
    to_pos = (int(move_str[2]) - 1, ord(move_str[3]) - ord('a'))

    # determine if the to piece is promoted
    if from_promoted:
        to_promoted = True
    else:
        if move_str[-1] == '+':
            to_promoted = True
        else:
            to_promoted = False

    # if the move is a capture, add the capture change
    to_idx = to_pos[0] * 9 + to_pos[1]
    if state.board[to_idx] is not None:
        opponent = 1 - player
        captured_type = state.board[to_idx].type
        captured_promoted = state.board[to_idx].promoted
        # captures have to be first in the change list
        move.append(
            Change(opponent, captured_type, captured_promoted, False, to_pos, None)
        )
    
    # add the main change
    move.append(Change(player, piece_type, from_promoted, to_promoted, from_pos, to_pos))

    # for change in move:
    #     print(change)

    return move

    
        
    


