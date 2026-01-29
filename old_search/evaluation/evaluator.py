from game.game_state import GameState
from game.misc import Change
from typing import List


class Evaluator:
    def evaluate(self, game_state: GameState) -> int:
        pass

    def evaluate_moves(self, moves: List[List[Change]], game_state: GameState) -> int:
        pass