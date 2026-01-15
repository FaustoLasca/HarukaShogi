from game.misc import Piece, Player, PieceType


PIECE_IMAGE_PATHS = {
    Piece(Player.BLACK, PieceType.KING): "assets/pieces/1GY.png",
    Piece(Player.WHITE, PieceType.KING): "assets/pieces/1OU.png",

    Piece(Player.BLACK, PieceType.GOLD): "assets/pieces/1KI.png",
    Piece(Player.WHITE, PieceType.GOLD): "assets/pieces/1KI.png",

    Piece(Player.BLACK, PieceType.SILVER, False): "assets/pieces/1GI.png",
    Piece(Player.WHITE, PieceType.SILVER, False): "assets/pieces/1GI.png",
    Piece(Player.BLACK, PieceType.SILVER, True): "assets/pieces/1NG.png",
    Piece(Player.WHITE, PieceType.SILVER, True): "assets/pieces/1NG.png",

    Piece(Player.BLACK, PieceType.KNIGHT, False): "assets/pieces/1KE.png",
    Piece(Player.WHITE, PieceType.KNIGHT, False): "assets/pieces/1KE.png",
    Piece(Player.BLACK, PieceType.KNIGHT, True): "assets/pieces/1NK.png",
    Piece(Player.WHITE, PieceType.KNIGHT, True): "assets/pieces/1NK.png",

    Piece(Player.BLACK, PieceType.LANCE, False): "assets/pieces/1KY.png",
    Piece(Player.WHITE, PieceType.LANCE, False): "assets/pieces/1KY.png",
    Piece(Player.BLACK, PieceType.LANCE, True): "assets/pieces/1NY.png",
    Piece(Player.WHITE, PieceType.LANCE, True): "assets/pieces/1NY.png",

    Piece(Player.BLACK, PieceType.BISHOP, False): "assets/pieces/1KA.png",
    Piece(Player.WHITE, PieceType.BISHOP, False): "assets/pieces/1KA.png",
    Piece(Player.BLACK, PieceType.BISHOP, True): "assets/pieces/1UM.png",
    Piece(Player.WHITE, PieceType.BISHOP, True): "assets/pieces/1UM.png",

    Piece(Player.BLACK, PieceType.ROOK, False): "assets/pieces/1HI.png",
    Piece(Player.WHITE, PieceType.ROOK, False): "assets/pieces/1HI.png",
    Piece(Player.BLACK, PieceType.ROOK, True): "assets/pieces/1RY.png",
    Piece(Player.WHITE, PieceType.ROOK, True): "assets/pieces/1RY.png",

    Piece(Player.BLACK, PieceType.PAWN, False): "assets/pieces/1FU.png",
    Piece(Player.WHITE, PieceType.PAWN, False): "assets/pieces/1FU.png",
    Piece(Player.BLACK, PieceType.PAWN, True): "assets/pieces/1TO.png",
    Piece(Player.WHITE, PieceType.PAWN, True): "assets/pieces/1TO.png",
}
