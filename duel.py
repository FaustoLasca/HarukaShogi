
import multiprocessing as mp
from collections import defaultdict

from controller.controller import Controller
from controller.player import Player, MinMaxPlayer, Haruka
from old_search.evaluation.piece_value import SimpleEvaluator

from starting_positions import STARTING_POSITIONS

import haruka
import haruka_v0_1
import haruka_v0_2


def duel(sfen: str = None) -> tuple[int, int]:
    n_wins = [0, 0]

    # play a game with player1 as black and player2 as white
    controller = Controller([PLAYER1, PLAYER2], None, sfen, max_moves=1000)
    winner = controller.run()
    if winner == 0:
        n_wins[0] += 1
    elif winner == 1:
        n_wins[1] += 1
    
    # play a game with player2 as black and player1 as white
    controller = Controller([PLAYER2, PLAYER1], None, sfen, max_moves=1000)
    winner = controller.run()
    if winner == 0:
        n_wins[1] += 1
    elif winner == 1:
        n_wins[0] += 1

    return tuple(n_wins)


haruka_v0_2.init()
searcher1 = haruka_v0_2.Searcher()
PLAYER1 = Haruka(time_limit=500, searcher=searcher1)

haruka_v0_1.init()
searcher2 = haruka_v0_1.Searcher()
PLAYER2 = Haruka(time_limit=500, searcher=searcher2)

NUM_PROCESSES = 30

if __name__ == "__main__":

    n = len(STARTING_POSITIONS)

    pool = mp.Pool(processes=NUM_PROCESSES)
    results = pool.map(duel, STARTING_POSITIONS)
    pool.close()

    n_wins = [0, 0]
    for wins1, wins2 in results:
        n_wins[0] += wins1
        n_wins[1] += wins2

    print(f"Results for player 1:")
    print(f"Wins: {n_wins[0]} - Draws: {n*2 - n_wins[0] - n_wins[1]} - Losses: {n_wins[1]}")