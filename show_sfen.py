from queue import Queue

from ui.gui import Gui
from game.game_state import GameState


SFEN_STRING = "ln6l/1r2gkg2/4ps1p1/p1pps1p1p/1p3N1n1/P1P1S1P1P/1PSPP4/2G2G3/LNK4RL w BPPbp 50"


state = GameState(SFEN_STRING)
ui_queue = Queue()
ui_queue.put((None, state.copy()))

ui = Gui(ui_queue, Queue(), Queue())
ui.run()