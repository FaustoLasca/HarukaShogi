from queue import Queue

from ui.gui import Gui
from game.game_state import GameState


SFEN_STRING = "ln1g5/1r3kg2/p2pppn+P1/2ps2p2/1p6l/2P6/PPSPPPPLN/2G2K1pP/LN4G1b w BSSPr 56"


state = GameState(SFEN_STRING)
ui_queue = Queue()
ui_queue.put((None, state.copy()))

ui = Gui(ui_queue, Queue(), Queue())
ui.run()