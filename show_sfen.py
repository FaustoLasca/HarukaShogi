from queue import Queue

from ui.gui import Gui
from game.game_state import GameState


SFEN_STRING = "1r4k1l/1P4gs1/4+Sp3/l1p4pp/PN1p3n1/1BP2P1GP/K3P4/SG+b6/r7L w GNNLPPPPsppp 112"

state = GameState(SFEN_STRING)
ui_queue = Queue()
ui_queue.put((None, state.copy()))

ui = Gui(ui_queue, Queue(), Queue())
ui.run()