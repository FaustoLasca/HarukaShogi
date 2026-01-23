from game.game_state import GameState
from time import time

def count_nodes(game_state: GameState, depth: int) -> int:
    if depth == 0 or game_state.is_game_over():
        return 1
    nodes = 0
    for move in game_state.generate_moves():
        game_state.move(move)
        nodes += count_nodes(game_state, depth - 1)
        game_state.unmove(move)
    return nodes

for depth in range(0, 4):
    game_state = GameState()
    start_time = time()
    nodes = count_nodes(game_state, depth)
    end_time = time()
    time_taken = end_time - start_time
    print(f"Depth {depth}: {nodes} nodes in {time_taken} seconds")
