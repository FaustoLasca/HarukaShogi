from typing import List
from queue import Queue
from typing import Optional

from game.game_state import GameState
from controller.player import Player


class Controller:
    def __init__(self, players: List[Player], update_ui_queue: Optional[Queue] = None, sfen: str = None, max_moves: int = 100000):
        self.state = GameState(sfen)
        self.update_ui_queue = update_ui_queue
        self.players = players
        self.max_moves = max_moves

    def run(self):
        for player in self.players:
            player.update_state(None, self.state.copy())
        if self.update_ui_queue is not None:
            self.update_ui_queue.put((None, self.state.copy()))

        n_moves = 0
        while not self.state.is_game_over() and n_moves < self.max_moves:
            moves = self.state.generate_moves()
            # handle stalemate here (edge case that should never happen)
            if len(moves) == 0:
                break
            current_player = self.state.current_player

            try:
                move = self.players[current_player].get_move(moves)

                self.state.move(move)
            except Exception as e:
                print(f"Error making move: {e}")
                break
            
            for player in self.players:
                player.update_state(move, self.state.copy())
            n_moves = n_moves + 1
            
            if self.update_ui_queue is not None:
                self.update_ui_queue.put((move, self.state.copy()))
        
        print(f"Game over. Winner: {self.state.winner}. Moves: {n_moves}")
        for player in self.players:
            player.close()
        
        return self.state.winner
            