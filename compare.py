import subprocess
from shogi import Board, BLACK, WHITE, Move
import time
import multiprocessing as mp
import threading
import queue


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
    path2 = "engines/HarukaShogi_v011"

    pool = mp.Pool(processes=30)

    results = {
        "win": 0,
        "loss": 0,
        "draw": 0,
        "error": 0,
    }

    args = [(path1, path2, 30000, False) for _ in range(150)] + \
           [(path1, path2, 30000, True) for _ in range(150)]

    outcomes = pool.starmap(play_game, args)

    for result in outcomes:
        results[result] += 1

    pool.close()

    print(results)