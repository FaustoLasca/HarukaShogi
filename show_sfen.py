from queue import Queue

from ui.gui import Gui
from game.game_state import GameState


SFEN_STRING = "7Gl/1psg1b3/l2k5/pP2PpPrp/1Np4n1/P5G2/3PGS2P/4K2P1/L6RL b SNNPPsbpppp 115"

state = GameState(SFEN_STRING)
ui_queue = Queue()
ui_queue.put((None, state.copy()))

ui = Gui(ui_queue, Queue(), Queue())
ui.run()