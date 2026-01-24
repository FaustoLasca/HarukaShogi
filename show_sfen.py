from queue import Queue

from ui.gui import Gui
from game.game_state import GameState


SFEN_STRING = "8l/lpsg1b3/3k5/pP2PpPrp/1Np4n1/P5G2/3PGS2P/4K2P1/L6RL b GSNNPPsbpppp 113"

state = GameState(SFEN_STRING)
ui_queue = Queue()
ui_queue.put((None, state.copy()))

ui = Gui(ui_queue, Queue(), Queue())
ui.run()