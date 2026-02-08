from queue import Queue

from ui.gui import Gui
from game.game_state import GameState


SFEN_STRING = "4k2nl/2g1g1+P2/+L3p3p/2ps1rNb1/4NpP2/pS1PP4/1PP1SP2P/K6+b1/3G3NL b GSLPPPPrp 85"


state = GameState(SFEN_STRING)
ui_queue = Queue()
ui_queue.put((None, state.copy()))

ui = Gui(ui_queue, Queue(), Queue())
ui.run()