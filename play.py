from typing import List
from queue import Queue
import threading

from controller.controller import Controller
from controller.player import Player, RandomPlayer, MiopicPlayer, MinMaxPlayer
from search.evaluation.piece_value import SimpleEvaluator
from search.evaluation.move_ordering import SimpleMoveOrderer
from ui.gui import Gui


def start_controller(update_ui_queue: Queue, players: List[Player]):
    controller = Controller(players, update_ui_queue)
    controller.run()

if __name__ == "__main__":
    update_ui_queue = Queue()
    players = [
        MiopicPlayer(SimpleEvaluator(), wait_time=0.5),
        MinMaxPlayer(SimpleEvaluator(), depth=3),
        # MinMaxPlayer(PieceValueEvaluator(), SimpleMoveOrderer(), depth=3),
    ]

    controller_thread = threading.Thread(target=start_controller, args=(update_ui_queue, players))
    controller_thread.start()

    ui = Gui(update_ui_queue, Queue(), Queue())
    ui.run()
