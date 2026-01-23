from typing import List
from queue import Queue
import threading

from controller.controller import Controller
from controller.player import Player, RandomPlayer, MiopicPlayer, MinMaxPlayer, GuiPlayer
from search.evaluation.piece_value import SimpleEvaluator
from search.evaluation.move_ordering import SimpleMoveOrderer
from ui.gui import Gui
from game.game_state import GameState


SFEN_STRING = "lnsgkgsnl/1r5b1/ppppp1ppp/5B3/9/2P4r1/PP1PPPP1P/9/LNSGKGSnl w Pp 6"

state = GameState(SFEN_STRING)
ui_queue = Queue()
ui_queue.put((None, state.copy()))

ui = Gui(ui_queue, Queue(), Queue())
ui.run()