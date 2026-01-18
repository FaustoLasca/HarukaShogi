from typing import List
from queue import Queue
from typing import Optional

from game.game_state import GameState
from controller.player import Player


class Controller:
    def __init__(self, players: List[Player], update_ui_queue: Optional[Queue] = None, sfen: str = None):
        self.state = GameState(sfen)
        self.update_ui_queue = update_ui_queue
        self.players = players

    def run(self):
        for player in self.players:
            player.update_state(None, self.state.copy())
        self.update_ui_queue.put((None, self.state.copy()))

        n_moves = 0
        while not self.state.is_game_over():
            moves = self.state.generate_moves()
            current_player = self.state.current_player
            move = self.players[current_player].get_move(moves)

            self.state.move(move)
            for player in self.players:
                player.update_state(move, self.state.copy())
            n_moves = n_moves + 1
            
            if self.update_ui_queue is not None:
                self.update_ui_queue.put((move, self.state.copy()))
        
        print(f"Game over. Winner: {self.state.winner}. Moves: {n_moves}")
        for player in self.players:
            player.close()
        
        return self.state.winner
            