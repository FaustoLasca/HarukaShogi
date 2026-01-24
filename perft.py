from game.game_state import GameState
from time import time
from searchengine import perft

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

DEPTH = 4
# SFEN = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1"
# SFEN = "ln6l/1r2gkg2/4psnp1/p1pps1p1p/1p3p3/P1P1S1P1P/1PSPP1N2/2G2G3/LNK4RL w BPbp 48"
SFEN = "ln1g5/1r3kg2/p2pppn+P1/2ps2p2/1p6l/2P6/PPSPPPPLN/2G2K1pP/LN4G1b w BSSPr 56"
# SFEN = "8l/lpsg1b3/3k5/pP2PpPrp/1Np4n1/P5G2/3PGS2P/4K2P1/L6RL b GSNNPPsbpppp 113"


print("Perft test at depth ", DEPTH)
print(f"sfen: {SFEN}\n")

for depth in range(0, DEPTH+1):
    game_state = GameState(SFEN)
    start_time = time()
    nodes = count_nodes(game_state, depth)
    cpp_nodes = perft(SFEN, depth)
    end_time = time()
    time_taken = end_time - start_time
    print(f"Depth {depth} \t| python: {nodes} - cpp: {cpp_nodes} - diff: {nodes - cpp_nodes} - {time_taken} s")

game_state = GameState(SFEN)

for move in game_state.generate_moves():
    game_state.move(move)
    nodes = count_nodes(game_state, DEPTH-1)
    cpp_nodes = perft(game_state.get_sfen(), DEPTH-1)
    print(f"python: {nodes} \t| cpp: {cpp_nodes} \t| diff: {nodes - cpp_nodes} \t| {game_state.get_sfen()}")
    game_state.unmove(move)
