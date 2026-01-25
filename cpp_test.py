import searchengine
from searchengine import perft, Searcher

searcher = Searcher()
searcher.set_position("ln5bl/1r2gkg2/4psnp1/p1pps1p1p/1p3p3/P1P1S1P1P/1PSPP1N2/2G2G3/LNK4RL b BPp 49")
print(searcher.search(time_limit=1000, max_depth=6))

# print(perft("ln5bl/1r2gkg2/4psnp1/p1pps1p1p/1p3p3/P1P1S1P1P/1PSPP1N2/2G2G3/LNK4RL b BPp 49", 4))