from typing import List
from queue import Queue
import threading

from controller.controller import Controller
from controller.player import Player, RandomPlayer, MiopicPlayer, MinMaxPlayer, GuiPlayer
from search.evaluation.piece_value import SimpleEvaluator
from search.evaluation.move_ordering import SimpleMoveOrderer
from ui.gui import Gui
from game.game_state import GameState


SFEN_STRING = "lnsgkg1nl/1r3+B1b1/1ppppp1pp/p8/9/2P6/PP1PPPPPP/7R1/LNSGKGSNL w SP 6"

state = GameState(SFEN_STRING)
ui_queue = Queue()
ui_queue.put((None, state.copy()))

ui = Gui(ui_queue, Queue(), Queue())
ui.run()