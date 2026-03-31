import subprocess
from shogi import Board, BLACK, WHITE, Move
import time
import multiprocessing as mp
import threading
import queue
import math


class EngineTimeout(Exception):
    pass


class Engine:
    def __init__(self, path: str):
        self.process = subprocess.Popen(
            path,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            text=True,
        )
        self._queue = queue.Queue()
        self._reader = threading.Thread(target=self._read_output, daemon=True)
        self._reader.start()
        self.send("setoption name Threads value 1")
        self.send("setoption name USI_Hash value 100")

    def _read_output(self):
        for line in self.process.stdout:
            self._queue.put(line.strip())

    def send(self, command: str):
        self.process.stdin.write(command + "\n")
        self.process.stdin.flush()

    def read_until(self, target: str, timeout: float = 10) -> list[str]:
        lines = []
        while True:
            try:
                line = self._queue.get(timeout=timeout)
            except queue.Empty:
                raise EngineTimeout(
                    f"Engine did not respond within {timeout}s (waiting for '{target}')"
                )
            lines.append(line)
            if line.startswith(target):
                break
        return lines

    def position(self, sfen: str):
        self.send(f"position sfen {sfen}")

    def go(self, **kwargs) -> str:
        tokens = ["go"]
        for key, value in kwargs.items():
            tokens.append(key)
            if value is not True:
                tokens.append(str(value))
        
        self.send(" ".join(tokens))
        lines = self.read_until("bestmove")
        return lines[-1].split()[1]

    def quit(self):
        try:
            self.send("quit")
            self.process.wait()
        except Exception as e:
            self.process.kill()
            self.process.wait()


def play_game(path1, path2, max_time, reverse):
    if not reverse:
        engine = {
            "black": Engine(path1),
            "white": Engine(path2),
        }
    else:
        engine = {
            "black": Engine(path2),
            "white": Engine(path1),
        }

    sfen = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1"
    board = Board(sfen)

    time_left = {BLACK: max_time, WHITE: max_time}

    engine["black"].send("newgame")
    engine["white"].send("newgame")

    num_moves = 0
    while num_moves < 500:
        if board.is_game_over():
            break

        t0 = time.perf_counter()
        try:
            if board.turn == BLACK:
                engine["black"].position(board.sfen())
                move = engine["black"].go(btime=time_left[BLACK], wtime=time_left[WHITE])
                time_left[BLACK] -= int((time.perf_counter() - t0) * 1000)
            else:
                engine["white"].position(board.sfen())
                move = engine["white"].go(btime=time_left[BLACK], wtime=time_left[WHITE])
                time_left[WHITE] -= int((time.perf_counter() - t0) * 1000)
        except Exception as e:
            engine["black"].quit()
            engine["white"].quit()
            return "error"

        board.push(Move.from_usi(move))
        num_moves += 1

    engine["black"].quit()	
    engine["white"].quit()

    result = "draw"
    if board.is_checkmate():
        result = "win" if board.turn == WHITE else "loss"
        if reverse:
            result = "loss" if result == "win" else "win"
    return result


if __name__ == "__main__":
    path1 = "searchengine/build/HarukaShogi"
    path2 = "engines/HarukaShogi_v015"

    MAX_GAMES = 3000
    NUM_PROCESSES = 30

    pool = mp.Pool(processes=NUM_PROCESSES)
        
    results = {
        "win": 0,
        "loss": 0,
        "draw": 0,
        "error": 0,
    }


    for i in range(math.floor(MAX_GAMES/(NUM_PROCESSES*10))):

        args = [(path1, path2, 30000, False) for _ in range(NUM_PROCESSES*5)] + \
            [(path1, path2, 30000, True) for _ in range(NUM_PROCESSES*5)]

        outcomes = pool.starmap(play_game, args)

        for outcome in outcomes:
            results[outcome] += 1

        N = results["win"] + results["loss"] + results["draw"]
        w = results["win"] / N
        l = results["loss"] / N
        d = results["draw"] / N

        win_ratio = (results["win"] + results["draw"] * 0.5) / N

        var = w * (1 - win_ratio)**2 + d * (0.5 - win_ratio)**2 + l * (-win_ratio)**2
        se = math.sqrt(var / N)

        # ~95% confidence interval
        p_lo = max(win_ratio - 2*se, 0.0001)
        p_hi = min(win_ratio + 2*se, 0.9999)

        elo = 400 * math.log10(win_ratio / (1 - win_ratio))
        elo_lo = 400 * math.log10(p_lo / (1 - p_lo))
        elo_hi = 400 * math.log10(p_hi / (1 - p_hi))

        los = 0.5 * (1 + math.erf((results["win"] - results["loss"]) / math.sqrt(2*results["win"] + 2*results["loss"])))

        print(f"RESULTS AFTER {(i+1)*NUM_PROCESSES*10} GAMES")
        print(f"wins - draws - losses - errors:   {results["win"]} - {results["draw"]} - {results["loss"]} - {results["error"]}")
        print(f"Win ratio:                        {win_ratio:.3f}")
        print(f"Elo delta:                        {round(elo)} [{round(elo_lo)} , {round(elo_hi)}]")
        print(f"Likelyhoodd of Superiority (LOS): {los:.3f}")
        print("---------------------------------------------------")

    pool.close()