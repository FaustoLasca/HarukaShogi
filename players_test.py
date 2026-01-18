from controller.player import RandomPlayer, MiopicPlayer, MinMaxPlayer
from search.evaluation.piece_value import PieceValueEvaluator
from search.evaluation.move_ordering import SimpleMoveOrderer
from controller.controller import Controller
from ui.gui import Gui
from queue import Queue
from time import time

players = [
    # RandomPlayer(),
    MiopicPlayer(PieceValueEvaluator()),
    MinMaxPlayer(PieceValueEvaluator(), SimpleMoveOrderer(), depth=3),
]

wins = [0, 0]
n_games = 10
total_time = 0
reversed_players = False
for i in range(n_games):
    controller = Controller(players)
    t0 = time()
    winner = controller.run()
    time_taken = time() - t0
    total_time += time_taken
    if winner is not None:
        wins[winner] += 1
    print(f"Game {i+1} took {time_taken} seconds")
    players = list(reversed(players))
    wins = list(reversed(wins))
    reversed_players = not reversed_players

if reversed_players:
    wins = list(reversed(wins))
    players = list(reversed(players))

print(f"Total time: {total_time} seconds")
print(f"Black wins: {wins[0]} ({wins[0] / n_games * 100}%)")
print(f"White wins: {wins[1]} ({wins[1] / n_games * 100}%)")
print(f"Draws: {n_games - wins[0] - wins[1]} ({((n_games - wins[0] - wins[1]) / n_games) * 100}%)")