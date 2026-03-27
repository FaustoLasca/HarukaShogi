from pathlib import Path
from shogi import Board, Move, KIF

kif_files = Path("kif_games").glob("*.kif")

with open("games.txt", "w") as f:
    for kif_file in kif_files:
        kif = KIF.Parser.parse_file(kif_file)[0]
        
        if kif["sfen"] == "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1":
            moves = [kif["moves"][i] for i in range(min(len(kif["moves"]), 40))]
            f.write(" ".join(moves) + "\n")