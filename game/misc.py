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

STARTING_PIECE_LIST = {
    Piece(Player.BLACK, PieceType.KING): {(4, 8)},
    Piece(Player.BLACK, PieceType.GOLD): {(3, 8), (5, 8)},
    Piece(Player.BLACK, PieceType.SILVER): {(2, 8), (6, 8)},
    Piece(Player.BLACK, PieceType.KNIGHT): {(1, 8), (7, 8)},
    Piece(Player.BLACK, PieceType.LANCE): {(0, 8), (8, 8)},
    Piece(Player.BLACK, PieceType.BISHOP): {(7, 7)},
    Piece(Player.BLACK, PieceType.ROOK): {(1, 7)},
    Piece(Player.BLACK, PieceType.PAWN): {(0, 6), (1, 6), (2, 6), (3, 6), (4, 6), (5, 6), (6, 6), (7, 6), (8, 6)},

    Piece(Player.WHITE, PieceType.KING): {(4, 0)},
    Piece(Player.WHITE, PieceType.GOLD): {(3, 0), (5, 0)},
    Piece(Player.WHITE, PieceType.SILVER): {(2, 0), (6, 0)},
    Piece(Player.WHITE, PieceType.KNIGHT): {(1, 0), (7, 0)},
    Piece(Player.WHITE, PieceType.LANCE): {(0, 0), (8, 0)},
    Piece(Player.WHITE, PieceType.BISHOP): {(1, 1)},
    Piece(Player.WHITE, PieceType.ROOK): {(7, 1)},
    Piece(Player.WHITE, PieceType.PAWN): {(0, 2), (1, 2), (2, 2), (3, 2), (4, 2), (5, 2), (6, 2), (7, 2), (8, 2)},
}