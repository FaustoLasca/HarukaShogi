from controller.player import RandomPlayer, MiopicPlayer, MinMaxPlayer
from search.evaluation.piece_value import SimpleEvaluator
from search.evaluation.move_ordering import SimpleMoveOrderer
from controller.controller import Controller
from ui.gui import Gui
from queue import Queue
from time import time
import random


random.seed(42)

players = [
    RandomPlayer(),
    MinMaxPlayer(SimpleEvaluator(), depth=4),
]

controller = Controller(players)
winner = controller.run()