from typing import List

from game.misc import Change
from game.game_state import GameState


class MoveOrderer:
    def order(self, moves: List[List[Change]], game_state: GameState) -> List[List[Change]]:
        pass


class SimpleMoveOrderer(MoveOrderer):
    def order(self, moves: List[List[Change]], game_state: GameState) -> List[List[Change]]:
        scores = []
        for move in moves:
            score = 0
            # captures gain score
            if len(move) > 1:
                score += 10
            
            # game_state.move(move)
            # is_check = game_state.is_check(1-game_state.current_player)
            # game_state.unmove(move)
            # if is_check:
            #     score += 20

            scores.append((score, move))

        scores.sort(key=lambda x: x[0], reverse=True)
        return [move for _, move in scores]