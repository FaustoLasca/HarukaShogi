from search.evaluation.evaluator import Evaluator

from game.game_state import GameState
from game.misc import PieceType, Player, Change
from typing import List


class SimpleEvaluator(Evaluator):
    def __init__(self):
        self.piece_values = {
            (PieceType.GOLD, False): 6,
            (PieceType.SILVER, False): 5,
            (PieceType.SILVER, True): 6,
            (PieceType.KNIGHT, False): 2,
            (PieceType.KNIGHT, True): 6,
            (PieceType.LANCE, False): 4,
            (PieceType.LANCE, True): 6,
            (PieceType.BISHOP, False): 10,
            (PieceType.BISHOP, True): 20,
            (PieceType.ROOK, False): 15,
            (PieceType.ROOK, True): 20,
            (PieceType.PAWN, False): 1,
            (PieceType.PAWN, True): 6,
        }
    
    def evaluate(self, game_state: GameState) -> int:
        if game_state.is_game_over():
            return 10000 if game_state.winner == game_state.current_player else -10000 if game_state.winner == 1 - game_state.current_player else 0
        
        score = 0

        # evaluate current player's pieces on the board
        for piece_type, promoted in game_state.piece_list[game_state.current_player].keys():
            if piece_type != PieceType.KING:
                score += self.piece_values[(piece_type, promoted)] * len(game_state.piece_list[game_state.current_player][(piece_type, promoted)])
        for piece_type, promoted in game_state.piece_list[1 - game_state.current_player].keys():
            if piece_type != PieceType.KING:
                score -= self.piece_values[(piece_type, promoted)] * len(game_state.piece_list[1 - game_state.current_player][(piece_type, promoted)])
        
        # evaluate current player's pieces in hand
        # hand pieces are worth more, since they can be placed anywhere
        for piece_type, count in game_state.hand[game_state.current_player].items():
            score += (self.piece_values[(piece_type, False)] + 2) * count
        for piece_type, count in game_state.hand[1 - game_state.current_player].items():
            score -= (self.piece_values[(piece_type, False)] + 2) * count

        # bonus for check
        if game_state.is_check(game_state.current_player):
            score += 10
        if game_state.is_check(1 - game_state.current_player):
            score -= 10

        return score

    
    def evaluate_moves(self, moves: List[List[Change]], game_state: GameState) -> int:
        scores = []
        for move in moves:
            score = 0
            # captures gain score
            if len(move) > 1:
                score += 10

            scores.append(score)
        return scores
