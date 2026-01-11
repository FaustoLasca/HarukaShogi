from enum import IntEnum, auto
from dataclasses import dataclass

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

@dataclass
class Piece:
    player: Player
    type: PieceType
    promoted: bool

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
    promote: bool
    from_pos: tuple[int, int]
    to_pos: tuple[int, int]