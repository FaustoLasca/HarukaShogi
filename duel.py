
import multiprocessing as mp
from collections import defaultdict

from controller.controller import Controller
from controller.player import Player, MinMaxPlayer, Haruka
from old_search.evaluation.piece_value import SimpleEvaluator

from starting_positions import STARTING_POSITIONS

import haruka
import haruka_v0_1
import haruka_v0_2
import haruka_v0_3
import haruka_v0_4
import haruka_v0_5
import haruka_v0_6
import haruka_v0_7
import haruka_v0_8
import haruka_v0_9
import haruka_v0_10


def duel(sfen: str = None) -> dict:

    player1 = Haruka(time_limit=50, useOpeningBook=True, module=haruka)
    player2 = Haruka(time_limit=50, useOpeningBook=True, module=haruka_v0_10)

    n_wins = [0, 0]
    errors = 0

    # play a game with player1 as black and player2 as white
    controller = Controller([player1, player2], None, sfen, max_moves=500)
    try:
        winner = controller.run()
        if winner == 0:
            n_wins[0] += 1
        elif winner == 1:
            n_wins[1] += 1
    except Exception as e:
        print(f"Error playing game: {e}")
        errors += 1
    
    
    # play a game with player2 as black and player1 as white
    controller = Controller([player2, player1], None, sfen, max_moves=500)
    try:
        winner = controller.run()
        if winner == 0:
            n_wins[1] += 1
        elif winner == 1:
            n_wins[0] += 1
    except Exception as e:
        print(f"Error playing game: {e}")
        errors += 1

    return dict(wins=n_wins[0], losses=n_wins[1], draws=2-n_wins[0]-n_wins[1]-errors, errors=errors)



NUM_PROCESSES = 4

if __name__ == "__main__":

    n = len(STARTING_POSITIONS)

    pool = mp.Pool(processes=NUM_PROCESSES)
    results = pool.map(duel, [None for _ in range(100)])
    pool.close()

    result = {}
    for key in results[0].keys():
        result[key] = sum(result[key] for result in results)
    
    print(f"Results for player 1:")
    for key, value in result.items():
        print(f"{key}: {value}")