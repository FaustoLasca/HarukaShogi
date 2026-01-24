from queue import Queue

from ui.gui import Gui
from game.game_state import GameState


SFEN_STRING = "1nsgkg1nl/lr3s1b1/pppppp+Bpp/9/9/2P6/PP1PPPPPP/7R1/LNSGKGSNL b P 5"

state = GameState(SFEN_STRING)
ui_queue = Queue()
ui_queue.put((None, state.copy()))

ui = Gui(ui_queue, Queue(), Queue())
ui.run()