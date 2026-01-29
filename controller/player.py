from typing import List
import time
import random
from queue import Queue

from game.game_state import GameState
from game.misc import Change, string_to_move
from game.misc import Player as PlayerEnum
from old_search.evaluation.evaluator import Evaluator
from old_search.evaluation.move_ordering import MoveOrderer
from old_search.alpha_beta.searcher import MinMaxSearcher

import haruka


class Player:
    def update_state(self, move: int, game_state: GameState) -> int:
        pass

    def get_move(self, available_moves: List[int]) -> int:
        pass

    def close(self):
        pass


class RandomPlayer(Player):
    def __init__(self, wait_time: float = None):
        self.wait_time = wait_time

    def get_move(self, available_moves: List[int]) -> int:
        if self.wait_time is not None:
            time.sleep(self.wait_time)
        return random.choice(available_moves)


class MiopicPlayer(Player):
    def __init__(self, evaluator: Evaluator, wait_time: float = None):
        self.wait_time = wait_time
        self.evaluator = evaluator

    def update_state(self, move: int, game_state: GameState) -> int:
        self.state = game_state

    def get_move(self, available_moves: List[int]) -> int:
        player = self.state.current_player
        best_score = -float('inf')
        best_moves = []
        for move in available_moves:
            self.state.move(move)
            score = - self.evaluator.evaluate(self.state)
            self.state.unmove(move)
            if score > best_score:
                best_score = score
                best_moves = [move]
            elif score == best_score:
                best_moves.append(move)
        
        if self.wait_time is not None:
            time.sleep(self.wait_time)
        return random.choice(best_moves)


class MinMaxPlayer(Player):
    def __init__(self, evaluator: Evaluator, max_depth: int = 20, time_budget: float = 600):
        self.evaluator = evaluator
        self.max_depth = max_depth
        self.time_budget = time_budget

    def update_state(self, move: int, game_state: GameState) -> int:
        self.state = game_state

    def get_move(self, available_moves: List[int]) -> int:
        searcher = MinMaxSearcher(self.state, self.evaluator)
        return searcher.search(self.max_depth, self.time_budget)


# Haruka uses the searchengine library to search for the best move
class Haruka(Player):
    def __init__(self, time_limit: int = 600000, max_depth: int = 20):
        haruka.init()
        
        self.time_limit = time_limit
        self.max_depth = max_depth
        self.searcher = haruka.Searcher()
        self.state = GameState()
    
    def update_state(self, move: int, game_state: GameState) -> int:
        self.state = game_state
        self.searcher.set_position(game_state.get_sfen())

    def get_move(self, available_moves: List[List[Change]]) -> List[Change]:
        move_str = self.searcher.search(self.time_limit, self.max_depth)
        return string_to_move(move_str, self.state)

    def close(self):
        self.searcher.print_stats()



class GuiPlayer(Player):
    def __init__(self,
        move_request_queue: Queue[List[Change]],
        move_response_queue: Queue[List[Change]]
    ) -> None:
        self.move_request_queue = move_request_queue
        self.move_response_queue = move_response_queue

    def get_move(self, available_moves: List[int]) -> int:
        self.move_request_queue.put(available_moves)
        return self.move_response_queue.get()