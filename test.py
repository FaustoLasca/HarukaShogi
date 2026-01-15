from game.game_state import GameState
from game.misc import Piece, Player, PieceType
from ui.gui import GUI

game_state = GameState()
gui = GUI(game_state)
gui.run()