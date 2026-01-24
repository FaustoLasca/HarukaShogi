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

    if nodes == 0:
        return 1
    return nodes

DEPTH = 3
SFEN = "ln5bl/1r2gkg2/4psnp1/p1pps1p1p/1p3p3/P1P1S1P1P/1PSPP1N2/2G2G3/LNK4RL b BPp 49"

for depth in range(0, DEPTH+1):
    game_state = GameState(SFEN)
    start_time = time()
    nodes = count_nodes(game_state, depth)
    end_time = time()
    time_taken = end_time - start_time
    print(f"Depth {depth}: {nodes} nodes in {time_taken} seconds")

game_state = GameState(SFEN)

for move in game_state.generate_moves():
    game_state.move(move)
    nodes = count_nodes(game_state, DEPTH-1)
    print(f"perft: {nodes} - sfen: {game_state.get_sfen()}")
    game_state.unmove(move)
