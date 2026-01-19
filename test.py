from search.alpha_beta.searcher import MinMaxSearcher
from search.evaluation.piece_value import SimpleEvaluator
from game.game_state import GameState
import random


random.seed(42)

SFEN_STRINGS = [
    None,
    "7nl/7k1/6Ppp/9/9/9/+p+p+p6/2+p6/K1+p6 b GG 1", # mate in 3
    "lnsg3nl/2k2sr2/pppp1pgpp/4p4/2P3pP1/9/PPSPPPP1P/2K2S1R1/LN1G1G1NL b Bb 21",
    "ln6l/1r2gkg2/4psnp1/p1pps1p1p/1p3p3/P1P1S1P1P/1PSPP1N2/2G2G3/LNK4RL w BPbp 48",
    "l4k2l/1r4gb1/p1+B1n1sp1/2p5p/PP4p2/2PSP1g2/3P4P/3GG1s2/L+p1K3RL b SNPPPPPnnp 81",
]


evaluator = SimpleEvaluator()
for SFEN_STRING in SFEN_STRINGS:
    print(f"Testing SFEN: {SFEN_STRING}")
    searcher = MinMaxSearcher(GameState(SFEN_STRING), evaluator)
    searcher.search(max_depth=4, time_budget=60)