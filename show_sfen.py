from queue import Queue

from ui.gui import Gui
from game.game_state import GameState


SFEN_STRING = "lnsgkg1nl/1r4sb1/ppppppBpp/9/9/2P6/PP1PPPPPP/7R1/LNSGKGSNL w P 4"


state = GameState(SFEN_STRING)
ui_queue = Queue()
ui_queue.put((None, state.copy()))

ui = Gui(ui_queue, Queue(), Queue())
ui.run()