from game.misc import Player, PieceType, Piece

# dictionary containing the move deltas for every piece
# doesn't include sliding moves
FIXED_MOVES_DICT = {
    # KING
    Piece(Player.BLACK, PieceType.KING, False): [(1, 1), (0, 1), (-1, 1), (1, 0), (-1, 0), (1, -1), (0, -1), (-1, -1)],
    Piece(Player.WHITE, PieceType.KING, False): [(1, 1), (0, 1), (-1, 1), (1, 0), (-1, 0), (1, -1), (0, -1), (-1, -1)],
    # GOLD
    Piece(Player.BLACK, PieceType.GOLD, False): [(0, 1), (1, 0), (-1, 0), (1, -1), (0, -1), (-1, -1)],
    Piece(Player.WHITE, PieceType.GOLD, False): [(1, 1), (0, 1), (-1, 1), (1, 0), (-1, 0), (0, -1)],
    # SILVER
    Piece(Player.BLACK, PieceType.SILVER, False): [(1, 1), (-1, 1), (1, -1), (0, -1), (-1, -1)],
    Piece(Player.WHITE, PieceType.SILVER, False): [(1, 1), (0, 1), (-1, 1), (1, -1), (-1, -1)],
    # KNIGHT
    Piece(Player.BLACK, PieceType.KNIGHT, False): [(1, -2), (-1, -2)],
    Piece(Player.WHITE, PieceType.KNIGHT, False): [(1, 2), (-1, 2)],
    # LANCE
    Piece(Player.BLACK, PieceType.LANCE, False): [],
    Piece(Player.WHITE, PieceType.LANCE, False): [],
    # BISHOP
    Piece(Player.BLACK, PieceType.BISHOP, False): [],
    Piece(Player.WHITE, PieceType.BISHOP, False): [],
    # ROOK
    Piece(Player.BLACK, PieceType.ROOK, False): [],
    Piece(Player.WHITE, PieceType.ROOK, False): [],
    # PAWN
    Piece(Player.BLACK, PieceType.PAWN, False): [(0, -1)],
    Piece(Player.WHITE, PieceType.PAWN, False): [(0, 1)],
    # Promoted pieces that act like a gold general
    Piece(Player.BLACK, PieceType.SILVER, True): [(0, 1), (1, 0), (-1, 0), (1, -1), (0, -1), (-1, -1)],
    Piece(Player.BLACK, PieceType.KNIGHT, True): [(0, 1), (1, 0), (-1, 0), (1, -1), (0, -1), (-1, -1)],
    Piece(Player.BLACK, PieceType.LANCE, True): [(0, 1), (1, 0), (-1, 0), (1, -1), (0, -1), (-1, -1)],
    Piece(Player.BLACK, PieceType.PAWN, True): [(0, 1), (1, 0), (-1, 0), (1, -1), (0, -1), (-1, -1)],
    Piece(Player.WHITE, PieceType.SILVER, True): [(1, 1), (0, 1), (-1, 1), (1, 0), (-1, 0), (0, -1)],
    Piece(Player.WHITE, PieceType.KNIGHT, True): [(1, 1), (0, 1), (-1, 1), (1, 0), (-1, 0), (0, -1)],
    Piece(Player.WHITE, PieceType.LANCE, True): [(1, 1), (0, 1), (-1, 1), (1, 0), (-1, 0), (0, -1)],
    Piece(Player.WHITE, PieceType.PAWN, True): [(1, 1), (0, 1), (-1, 1), (1, 0), (-1, 0), (0, -1)],
    # other promoted pieces
    Piece(Player.BLACK, PieceType.BISHOP, True): [(1, 0), (0, 1), (-1, 0), (0, -1)],
    Piece(Player.WHITE, PieceType.BISHOP, True): [(1, 0), (0, 1), (-1, 0), (0, -1)],
    Piece(Player.BLACK, PieceType.ROOK, True): [(1, 1), (-1, 1), (1, -1), (-1, -1)],
    Piece(Player.WHITE, PieceType.ROOK, True): [(1, 1), (-1, 1), (1, -1), (-1, -1)],
}

SLIDING_MOVES_DICT = {
    Piece(Player.BLACK, PieceType.BISHOP, False): [(1, 1), (-1, 1), (1, -1), (-1, -1)],
    Piece(Player.WHITE, PieceType.BISHOP, False): [(1, 1), (-1, 1), (1, -1), (-1, -1)],
    Piece(Player.BLACK, PieceType.BISHOP, True): [(1, 1), (-1, 1), (1, -1), (-1, -1)],
    Piece(Player.WHITE, PieceType.BISHOP, True): [(1, 1), (-1, 1), (1, -1), (-1, -1)],
    Piece(Player.BLACK, PieceType.ROOK, False): [(1, 0), (0, 1), (-1, 0), (0, -1)],
    Piece(Player.WHITE, PieceType.ROOK, False): [(1, 0), (0, 1), (-1, 0), (0, -1)],
    Piece(Player.BLACK, PieceType.ROOK, True): [(1, 0), (0, 1), (-1, 0), (0, -1)],
    Piece(Player.WHITE, PieceType.ROOK, True): [(1, 0), (0, 1), (-1, 0), (0, -1)],

    Piece(Player.BLACK, PieceType.LANCE, False): [(0, -1)],
    Piece(Player.WHITE, PieceType.LANCE, False): [(0, 1)],
}