from game.game_state import GameState
from time import time
from haruka import perft, init

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
# SFEN = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1"
SFEN = "ln6l/1r2gkg2/4psnp1/p1pps1p1p/1p3p3/P1P1S1P1P/1PSPP1N2/2G2G3/LNK4RL w BPbp 48"
# SFEN = "ln1g5/1r3kg2/p2pppn+P1/2ps2p2/1p6l/2P6/PPSPPPPLN/2G2K1pP/LN4G1b w BSSPr 56"
# SFEN = "8l/lpsg1b3/3k5/pP2PpPrp/1Np4n1/P5G2/3PGS2P/4K2P1/L6RL b GSNNPPsbpppp 113"
# SFEN = "l3+B2Sl/1+r2k4/p2g2+Bpp/2p1ppN2/1l1pPP3/2Pn5/P1S5P/1P1G5/LKN5+r w GSPgsnpppp 124"
# SFEN = "3gk2nl/4g1+P2/+L3p3p/2ps1rNb1/4NpP2/pS1PP4/1PP1SP2P/K6+b1/3G3NL w GSLPPPPrp 84"
# SFEN = "1r4k1l/1P4gs1/4+Sp3/l1pB3pp/PN1p3n1/2P2P1GP/K2+bP4/SG7/r7L w GNNLPPPPsppp 110"

# SFEN = "1r4k1l/1P4gs1/4+Sp3/l1pB3pp/PN1p3n1/2P2P1GP/K3P4/SG+b6/r7L b GNNLPPPPsppp 111"
# SFEN = "1r4k1l/1P4gs1/4+Sp3/l1p4pp/PN1p3n1/1BP2P1GP/K3P4/SG+b6/r7L w GNNLPPPPsppp 112"

# SFEN = "ln1g3nl/2s1k1gsp/3ppp1p1/p1p3p1P/1P7/PG1PR4/2NKPPP1L/6S2/L+pb2G1N1 w RBPPs 60"
# SFEN = "l+R1g3nl/2n1k1gs1/3ppp1pp/1Pps2P2/2P3S2/P1B6/2gPPP2P/2B1KL+r2/7NL b SNPgppp 81"
# SFEN = "lnbg4l/1ks6/pppp2g1p/2+r2s+S2/4Pp3/2P4P1/PPSP2R1P/2K2G3/LNB1G3L w NPPPPnp 86"

init()

for depth in range(0, DEPTH+1):
    game_state = GameState(SFEN)

    start_time = time()

    nodes = count_nodes(game_state, depth)
    cpp_nodes = perft(SFEN, depth)

    end_time = time()
    time_taken = end_time - start_time

    print(f"Depth {depth} \t| python: {nodes} - cpp: {cpp_nodes} - diff: {nodes - cpp_nodes} - {time_taken} s")


game_state = GameState(SFEN)

total_p_nodes = 0
total_cpp_nodes = 0

for move in game_state.generate_moves():
    game_state.move(move)

    nodes = count_nodes(game_state, DEPTH-1)
    cpp_nodes = perft(game_state.get_sfen(), DEPTH-1)

    total_p_nodes += nodes
    total_cpp_nodes += cpp_nodes

    print(f"python: {nodes} \t| cpp: {cpp_nodes} \t| diff: {nodes - cpp_nodes} \t| {game_state.get_sfen()}")
    game_state.unmove(move)

print("Total python nodes: ", total_p_nodes)
print("Total cpp nodes: ", total_cpp_nodes)
print("Total diff: ", total_p_nodes - total_cpp_nodes)