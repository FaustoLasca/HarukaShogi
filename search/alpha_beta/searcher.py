from game.game_state import GameState
from game.misc import Player, Change
from search.evaluation.evaluator import Evaluator
from search.evaluation.move_ordering import MoveOrderer

from typing import List
import time


class MinMaxSearcher:
    def __init__(self, game_state: GameState, evaluator: Evaluator):
        self.game_state = game_state
        self.evaluator = evaluator
        self.node_count = 0

        # principal variation table to easily get the best move for a given depth
        # and keep track af the principal variation with iterative deepening
        self.pv_table: List[List[List[Change]]] = [[None] * 10 for _ in range(10)]
        self.pv_length = [0] * 11   # needs to be one longer than the depth to avoid index out of bounds
        self.following_pv = False


    def search(self, max_depth: int = 10, time_budget: float = 1000000) -> int:
        self.node_count = 0
        t0 = time.time()
        t_limit = t0 + time_budget

        # reset the pv table (only need to reset the length, the moves are overwritten)
        self.pv_length = [0] * 11

        # iterative deepening, using the pv table to store the best moves
        for d in range(1, max_depth + 1):
            self.following_pv = True
            try:
                evaluation = self.min_max(d, 0, -float('inf'), float('inf'), t_limit)
            except Exception as e:
                break
            depth = d
            if time.time() > t_limit:
                break

        d_time = time.time() - t0
        print(f"Search finished.\t Evaluation: {evaluation}. \t Depth: {depth}.\t Node count: {self.node_count}.\t Time taken: {d_time} seconds.\t Nodes per second: {self.node_count / d_time}")
        return self.pv_table[0][0]

    
    def min_max(self, depth: int, ply: int, alpha: int, beta: int, time_limit: float) -> int:
        # If the time budget is exceeded raise an exception
        if time.time() > time_limit:
            raise Exception("Time limit exceeded")

        self.node_count += 1
        # if a leaf node is reached return the evaluation
        if depth == 0 or self.game_state.is_game_over():
            self.pv_length[ply] = 0
            return self.evaluator.evaluate(self.game_state)

        # order moves to put more promising moves first
        # this maximizes pruning
        moves = self.game_state.generate_moves()
        # fix for stalemate
        # this check isn't made in is_game_over() because it's expensive
        if len(moves) == 0:
            return 0
        move_scores = self.evaluator.evaluate_moves(moves, self.game_state)
        moves = [move for _, move in sorted(zip(move_scores, moves), key=lambda x: x[0], reverse=True)]

        # if we are following the pv moves, put the move in the pv_table first
        if self.following_pv:
            if self.pv_length[0] > ply:
                pv_move = self.pv_table[0][ply]
                moves.remove(pv_move)
                moves.insert(0, pv_move)
            else:
                self.following_pv = False


        # explore the next moves and return the best evaluation
        best_evaluation = -float('inf')
        for move in moves:
            # evaluate the node
            self.game_state.move(move)
            # we invert and negate the alpha and beta values for the next move (negamax)
            # the alpha and beta values have their roles reversed for the opponent
            evaluation = -self.min_max(depth - 1, ply + 1, -beta, -alpha, time_limit)
            self.game_state.unmove(move)

            # Check if move is the best so far
            # alpha acts as the best evaluation for the current player
            # beta acts as the best evaluation for the opponent
            if evaluation > best_evaluation:
                best_evaluation = evaluation
                alpha = max(alpha, best_evaluation)

                # update the pv table with the new best variation for this ply
                # the first element of the pv table is the best move for this ply
                # the rest are the principal variation for the next ply
                self.pv_table[ply][0] = move
                for i in range(self.pv_length[ply + 1]):
                    self.pv_table[ply][i + 1] = self.pv_table[ply + 1][i]
                self.pv_length[ply] = self.pv_length[ply + 1] + 1

            # if alpha is greater than beta, prune the search
            # the opponent will not choose this path either way
            if alpha >= beta:
                return best_evaluation
        
        return best_evaluation

            