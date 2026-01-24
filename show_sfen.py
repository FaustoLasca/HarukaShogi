from queue import Queue

from ui.gui import Gui
from game.game_state import GameState


SFEN_STRING = "1n1g5/lr3kg2/p2pppn+P1/2ps2p1B/1p7/2P5l/PPSPPPPLN/2G2K1pP/LN4G1b b SSPr 59"

state = GameState(SFEN_STRING)
ui_queue = Queue()
ui_queue.put((None, state.copy()))

ui = Gui(ui_queue, Queue(), Queue())
ui.run()