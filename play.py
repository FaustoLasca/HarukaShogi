from typing import List
from queue import Queue
import threading

from controller.controller import Controller
from controller.player import Player, RandomPlayer, MiopicPlayer, MinMaxPlayer
from search.evaluation.piece_value import SimpleEvaluator
from search.evaluation.move_ordering import SimpleMoveOrderer
from ui.gui import Gui


SFEN_STRING = None
# SFEN_STRING = "7nl/7k1/6Ppp/9/9/9/+p+p+p6/2+p6/K1+p6 b GG 1" # mate in 3
# SFEN_STRING = "7nl/7k1/5+Pppp/9/9/9/+p+p+p6/2+p6/K1+p6 b brppSG 1" # mate in 3
# SFEN_STRING = "5b1nl/4g1sk1/4ppppp/9/4B4/9/+p+p+p6/2+p6/K1+p6 b GN 1" # mate in 3
# SFEN_STRING = "9/9/2R1g4/S5P2/4k1+l2/9/3+P2+P2/9/8K b - 1" # mate in 5
# SFEN_STRING = "4b3g/6P1R/6BkS/7g1/9/9/9/9/8K b P 1" # mate in 5
# SFEN_STRING = "3Ggk1n+P/4spPn+P/5G3/9/9/9/9/9/8K b - 1" # mate in 5
# SFEN_STRING = "2b1g4/3+B1k1+P1/6N2/6P2/9/9/9/9/8K b S 1" # mate in 5
# SFEN_STRING = "3lkg3/2+Bnnp3/3+L5/9/9/9/9/9/8K b S 1" # mate in 5
# SFEN_STRING = "9/6B2/2+P2S3/3p1p3/2gk2n1+B/9/3S+p1G2/9/K6N1 b - 1" # mate in 5
SFEN_STRING = "7nl/5R1gk/6Ppp/9/9/9/+p+p+p6/2+p6/K1+p6 b G 1" # mate in 5
# SFEN_STRING = "4R2nl/6sk1/6pp1/8p/9/9/+p+p+p6/2+p6/K1+p6 b GLB 1" # mate in 5




def start_controller(update_ui_queue: Queue, players: List[Player], sfen: str = None):
    controller = Controller(players, update_ui_queue, sfen)
    controller.run()

if __name__ == "__main__":
    update_ui_queue = Queue()
    players = [
        MinMaxPlayer(SimpleEvaluator(), depth=5),
        MinMaxPlayer(SimpleEvaluator(), depth=5),
    ]

    controller_thread = threading.Thread(target=start_controller, args=(update_ui_queue, players), kwargs={'sfen': SFEN_STRING})
    controller_thread.start()

    ui = Gui(update_ui_queue, Queue(), Queue())
    ui.run()
