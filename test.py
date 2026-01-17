import threading
import multiprocessing as mp
from queue import Queue
import time
import random

from game.game_state import GameState
from game.misc import Piece, Player, PieceType, Change
from ui.gui import GUI


def check_board(game_state: GameState):
    for player in Player:
        for piece_type, promoted in game_state.piece_list[player]:
            for from_col, from_row in game_state.piece_list[player][(piece_type, promoted)]:
                piece = Piece(player, piece_type, promoted)
                if game_state.board[from_col*9 + from_row] != piece:
                    print(game_state.board[from_col*9 + from_row], piece)
                assert game_state.board[from_col*9 + from_row] == piece

def start_controller(update_queue: Queue):
    game_state = GameState()
    game_state.move([
        Change(Player.BLACK, PieceType.SILVER, False, False, (2, 8), None),
        Change(Player.BLACK, PieceType.KNIGHT, False, False, (1, 8), None),
        Change(Player.BLACK, PieceType.LANCE, False, False, (0, 8), None),
        Change(Player.BLACK, PieceType.PAWN, False, False, (6, 6), None),
        Change(Player.WHITE, PieceType.PAWN, False, False, (0, 2), None),
        Change(Player.WHITE, PieceType.PAWN, False, False, (4, 2), None),
        Change(Player.BLACK, PieceType.BISHOP, False, False, (7, 7), (4, 2)),
    ])
    update_queue.put(([], game_state))
    
    while True:
        moves = game_state.generate_pseudo_legal_moves()
        # for move in moves:
        #     # show all moves
        #     time.sleep(0.2)
        #     print("Making move: ", move)
        #     game_state.move(move)
        #     check_board(game_state)
        #     update_queue.put((move, game_state))
        #     time.sleep(0.2)
        #     print("Unmaking move")
        #     game_state.unmove(move)
        #     check_board(game_state)
        #     update_queue.put((move, game_state))

        # choose a random move
        # time.sleep(0.1)
        move = random.choice(moves)
        game_state.move(move)
        check_board(game_state)
        update_queue.put((move, game_state))
        # print("Finished moves")


if __name__ == "__main__":
    mp.set_start_method("spawn")

    update_queue = Queue()

    controller_thread = threading.Thread(
        target=start_controller,
        args=(update_queue,)
        )
    controller_thread.start()

    gui = GUI(update_queue, Queue(), Queue())
    gui.run()