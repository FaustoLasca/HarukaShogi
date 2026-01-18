from game.game_state import GameState
from game.misc import Player
from search.evaluation.evaluator import Evaluator
from search.evaluation.move_ordering import MoveOrderer

import time


class MinMaxSearcher:
    def __init__(self, game_state: GameState, evaluator: Evaluator, move_orderer: MoveOrderer):
        self.game_state = game_state
        self.evaluator = evaluator
        self.move_orderer = move_orderer
        self.best_move = None
        self.node_count = 0


    def search(self, depth: int) -> int:
        self.node_count = 0
        t0 = time.time()
        self.min_max(depth, 0, -float('inf'), float('inf'))
        d_time = time.time() - t0
        print(f"Search finished. Node count: {self.node_count}. Time taken: {d_time} seconds. Nodes per second: {self.node_count / d_time}")
        return self.best_move
    
    def min_max(self, depth: int, ply: int, alpha: int, beta: int) -> int:
        self.node_count += 1
        current_player = self.game_state.current_player
        # if a leaf node is reached return the evaluation
        if depth == 0 or self.game_state.is_game_over():
            return self.evaluator.evaluate(self.game_state)

        # order moves to put more promising moves first
        # this maximizes pruning
        moves = self.game_state.generate_moves()
        moves = self.move_orderer.order(moves, self.game_state)


        # explore the next moves and return the best evaluation
        best_evaluation = -float('inf')
        for move in moves:
            # evaluate the node
            self.game_state.move(move)
            # we invert and negate the alpha and beta values for the next move (negamax)
            # the alpha and beta values have their roles reversed for the opponent
            evaluation = -self.min_max(depth - 1, ply + 1, -beta, -alpha)
            self.game_state.unmove(move)

            # Check if move is the best so far
            # alpha acts as the best evaluation for the current player
            # beta acts as the best evaluation for the opponent
            if evaluation > best_evaluation:
                best_evaluation = evaluation
                best_move = move
                alpha = max(alpha, best_evaluation)
            # if alpha is greater than beta, prune the search
            # the opponent will not choose this path either way
            if alpha >= beta:
                return best_evaluation
        
        if ply == 0:
            self.best_move = best_move
        
        return best_evaluation

            