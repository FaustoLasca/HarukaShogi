from controller.player import RandomPlayer, MiopicPlayer, MinMaxPlayer
from search.evaluation.piece_value import PieceValueEvaluator
from search.evaluation.move_ordering import SimpleMoveOrderer
from controller.controller import Controller
from ui.gui import Gui
from queue import Queue
from time import time

players = [
    RandomPlayer(),
    MinMaxPlayer(PieceValueEvaluator(), SimpleMoveOrderer(), depth=3),
]

controller = Controller(players)
winner = controller.run()