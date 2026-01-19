from typing import List
import time
import random
from queue import Queue

from game.game_state import GameState
from game.misc import Change
from game.misc import Player as PlayerEnum
from search.evaluation.evaluator import Evaluator
from search.evaluation.move_ordering import MoveOrderer
from search.alpha_beta.searcher import MinMaxSearcher


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
    def __init__(self, evaluator: Evaluator, max_depth: int, time_budget: float):
        self.evaluator = evaluator
        self.max_depth = max_depth
        self.time_budget = time_budget

    def update_state(self, move: int, game_state: GameState) -> int:
        self.state = game_state

    def get_move(self, available_moves: List[int]) -> int:
        searcher = MinMaxSearcher(self.state, self.evaluator)
        return searcher.search(self.max_depth, self.time_budget)


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