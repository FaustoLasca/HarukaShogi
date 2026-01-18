from collections import defaultdict
import copy
from typing import List

from game.move_rules import FIXED_MOVES_DICT, SLIDING_MOVES_DICT
from game.misc import Player, PieceType, Piece, PROMOTABLE, Change

class GameState:
    def __init__(self, sfen: str = None):
        self.board : List[Piece | None] = [None] * 81
        self.hand = {
            Player.BLACK: defaultdict[PieceType, int](int),
            Player.WHITE: defaultdict[PieceType, int](int),
        }
        # to make move generation faster
        self.piece_list = {
            Player.BLACK: defaultdict[tuple[PieceType, bool], set[tuple[int, int]]](set),
            Player.WHITE: defaultdict[tuple[PieceType, bool], set[tuple[int, int]]](set),
        }
        self.current_player = Player.BLACK
        self.game_over = None
        self.winner = None
        self.move_count = 0
        # cached information to avoid recomputing
        # temporary flag to indicate that we made a temporary move
        self.temporary_move = False
        self.legal_moves = None
        self.check_cache = {
            Player.BLACK: None,
            Player.WHITE: None,
        }

        self.set_state(sfen)


    def copy(self) -> 'GameState':
        return copy.deepcopy(self)


    def is_game_over(self) -> bool:
        # TODO: stalemate is not detected
        if self.game_over is not None:
            return self.game_over
        # if the king is in check, verify for checkmate

        if self.is_check(1-self.current_player):
            self.generate_moves()
            return self.game_over
            
        else:
            self.game_over = False
            return self.game_over


    def get_winner(self) -> Player | None:
        if self.game_over is not None:
            return self.winner
        # generating moves will set the winner flag and save the legal moves
        self.is_game_over()
        return self.winner


    def set_state(self, sfen: str = None):
        """
        Initializes the game starting from a given SFEN string.
        If no SFEN string is provided, the starting position is used.
        """
        # initialize an empty board and hands
        self.board = [None] * 81
        self.hand = {
            Player.BLACK: defaultdict[PieceType, int](int),
            Player.WHITE: defaultdict[PieceType, int](int),
        }
        self.piece_list = {
            Player.BLACK: defaultdict[tuple[PieceType, bool], set[tuple[int, int]]](set),
            Player.WHITE: defaultdict[tuple[PieceType, bool], set[tuple[int, int]]](set),
        }

        if sfen is None:
            sfen = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1"
        
        # parse the SFEN string into parts
        # the parts are:
        # 1. the board position
        # 2. the side to move
        # 3. the pieces in the hands
        # 4. the move count
        sfen_parts = sfen.strip().split(' ')
        piece_map = {
                    'k': PieceType.KING,
                    'g': PieceType.GOLD,
                    's': PieceType.SILVER,
                    'n': PieceType.KNIGHT,
                    'l': PieceType.LANCE,
                    'b': PieceType.BISHOP,
                    'r': PieceType.ROOK,
                    'p': PieceType.PAWN,
                }

        # parse the SFEN string and add the pieces as i do it
        # the SFEN string starts from the top left corner and goes to the bottom right corner
        col = 8
        row = 0
        promoted = False
        for char in sfen_parts[0]:
            # if the character is a digit, it means that we need to skip that many squares
            if char.isdigit():
                col -= int(char)
            
            # if the character is /, we need to go to the next row
            elif char == '/':
                row += 1
                col = 8

            # if the character is +, the next piece is promoted
            elif char=='+':
                promoted = True
            
            # if the character is a letter, it means that we need to add a piece to the board and update the piece list
            elif char.isalpha():
                if char.isupper():
                    player = Player.BLACK
                else:
                    player = Player.WHITE

                piece_type = piece_map[char.lower()]

                self.board[col*9 + row] = Piece(player, piece_type, promoted)
                self.piece_list[player][(piece_type, promoted)].add((col, row))
                col -= 1
                promoted = False
            
            else:
                raise ValueError(f"Invalid character \'{char}\' in SFEN string")
            
        # parse the side to move
        self.current_player = Player.BLACK if sfen_parts[1] == 'b' else Player.WHITE

        # parse the pieces in the hands
        for char in sfen_parts[2]:
            # no pieces in the hands
            if char == '-':
                break
            
            player = Player.BLACK if char.isupper() else Player.WHITE
            piece_type = piece_map[char.lower()]
            self.hand[player][piece_type] += 1

        # parse the move count
        self.move_count = int(sfen_parts[3]) - 1

        # check if the game is over
        # this will set the game_over and winner flags
        self.is_game_over()
    

    def generate_moves(self):
        # if the legal moves have already been generated, return them
        if not self.temporary_move and self.legal_moves is not None:
            return self.legal_moves
        # generate pseudo legal moves
        player = self.current_player
        pseudo_legal_moves = self.generate_pseudo_legal_moves()
        # filter illegal moves (player's king is in check)
        king_square = next(iter(self.piece_list[player][(PieceType.KING, False)]))
        king_in_check = self.is_square_attacked_by_sliding(1-player, king_square)
        if not king_in_check:
            king_in_check = self.is_check(1-player)
        if not king_in_check:
            possible_discovered_check = self.is_square_attacked_by_sliding(1-player, king_square, x_ray=True)
        legal_moves = []
        for move in pseudo_legal_moves:
            # the move needs to fully control for check only if:
            # - the king is in check
            # - the king moved
            # in other situations we only need to verify for discovered checks

            # check if the move is a king move
            king_move = False
            for change in move:
                if change.piece_type == PieceType.KING:
                    king_move = True
                    break
            # check if the move is a drop
            drop = move[0].from_pos is None
            
            # if the king is in check or the move is a king move, we need to fully verify for check
            if king_in_check or king_move:
                self.move_temporary(move)
                if not self.is_check(1-player):
                    legal_moves.append(move)
                self.unmove_temporary(move)
            
            # if the move is not a drop, we need to check for discovered checks
            # only applies if there is a possible discovered check
            elif not drop and possible_discovered_check:
                self.move_temporary(move)
                king_square = next(iter(self.piece_list[player][(PieceType.KING, False)]))
                if not self.is_square_attacked_by_sliding(1-player, king_square):
                    legal_moves.append(move)
                self.unmove_temporary(move)
            
            # if the move is a drop, it can't be a check
            else:
                legal_moves.append(move)

        # if there are no legal moves, the game is over
        if len(legal_moves) == 0:
            self.game_over = True
            if self.is_check(1-player):
                # checkmate
                self.winner = 1 - player
            else:
                # stalemate
                self.winner = None
        
        if not self.temporary_move:
            self.legal_moves = legal_moves
        return legal_moves
    

    def generate_pseudo_legal_moves(self) -> List[List[Change]]:
        moves = []

        # generate moves for pieces on the board
        for piece_type, promoted in self.piece_list[self.current_player].keys():
            for from_col, from_row in self.piece_list[self.current_player][(piece_type, promoted)]:
                piece = Piece(self.current_player, piece_type, promoted)
                # check fixed moves first
                for delta_col, delta_row in FIXED_MOVES_DICT[piece]:
                    to_col = from_col + delta_col
                    to_row = from_row + delta_row
                    # check if the move is on the board
                    if 0 <= to_col < 9 and 0 <= to_row < 9:
                        forced_promotion = not promoted and (
                                (piece_type == PieceType.PAWN and self.current_player == Player.BLACK and to_row == 0) or 
                                (piece_type == PieceType.PAWN and self.current_player == Player.WHITE and to_row == 8) or
                                (piece_type == PieceType.KNIGHT and self.current_player == Player.BLACK and to_row <= 1) or
                                (piece_type == PieceType.KNIGHT and self.current_player == Player.WHITE and to_row >= 7)
                            )
                        # check if the square is empty
                        if self.board[to_col*9 + to_row] is None:
                            if not forced_promotion:
                                moves.append(
                                    [
                                        Change(self.current_player, piece_type, promoted, promoted, (from_col, from_row), (to_col, to_row))
                                    ]
                                )
                            # check if the piece can be promoted
                            if piece_type in PROMOTABLE and not promoted:
                                if (self.current_player == Player.BLACK and (to_row < 3 or from_row < 3)) or (self.current_player == Player.WHITE and (to_row >= 6 or from_row >= 6)):
                                    moves.append(
                                        [
                                            Change(self.current_player, piece_type, promoted, True, (from_col, from_row), (to_col, to_row))
                                        ]
                                    )
                        # check if the square is occupied by the opponent
                        elif self.board[to_col*9 + to_row].player != self.current_player:
                            captured_piece = self.board[to_col*9 + to_row]
                            if not forced_promotion:
                                moves.append(
                                    [
                                        Change(captured_piece.player, captured_piece.type, captured_piece.promoted, False, (to_col, to_row), None),
                                        Change(self.current_player, piece_type, promoted, promoted, (from_col, from_row), (to_col, to_row))
                                    ]
                                )
                            if piece_type in PROMOTABLE and not promoted:
                                if (self.current_player == Player.BLACK and (to_row < 3 or from_row < 3)) or (self.current_player == Player.WHITE and (to_row >= 6 or from_row >= 6)):
                                    moves.append(
                                        [
                                            Change(captured_piece.player, captured_piece.type, captured_piece.promoted, False, (to_col, to_row), None),
                                            Change(self.current_player, piece_type, promoted, True, (from_col, from_row), (to_col, to_row))
                                        ]
                                    )
                # check sliding moves
                if piece in SLIDING_MOVES_DICT:
                    for delta_col, delta_row in SLIDING_MOVES_DICT[piece]:
                        to_col = from_col
                        to_row = from_row
                        while 0 <= to_col + delta_col < 9 and 0 <= to_row + delta_row < 9:
                            to_col += delta_col
                            to_row += delta_row
                            forced_promotion = not promoted and (
                                    (piece_type == PieceType.LANCE and self.current_player == Player.BLACK and to_row == 0) or
                                    (piece_type == PieceType.LANCE and self.current_player == Player.WHITE and to_row == 8)
                                )
                            # if the square is empty, add the move and continue
                            if self.board[to_col*9 + to_row] is None:
                                if not forced_promotion:
                                    moves.append(
                                        [
                                            Change(self.current_player, piece_type, promoted, promoted, (from_col, from_row), (to_col, to_row))
                                        ]
                                    )
                                # check if the piece can be promoted
                                if not promoted:
                                    if (self.current_player == Player.BLACK and (to_row < 3 or from_row < 3)) or (self.current_player == Player.WHITE and (to_row >= 6 or from_row >= 6)):
                                        moves.append(
                                            [
                                                Change(self.current_player, piece_type, promoted, True, (from_col, from_row), (to_col, to_row))
                                            ]
                                        )
                            # if the square is occupied by the opponent add the move and break
                            elif self.board[to_col*9 + to_row].player != self.current_player:
                                captured_piece = self.board[to_col*9 + to_row]
                                if not forced_promotion:
                                    moves.append(
                                        [
                                            Change(captured_piece.player, captured_piece.type, captured_piece.promoted, False, (to_col, to_row), None),
                                            Change(self.current_player, piece_type, promoted, promoted, (from_col, from_row), (to_col, to_row))
                                        ]
                                    )
                                # check if the piece can be promoted
                                if not promoted:
                                    if (self.current_player == Player.BLACK and (to_row < 3 or from_row < 3)) or (self.current_player == Player.WHITE and (to_row >= 6 or from_row >= 6)):
                                        moves.append(
                                            [
                                                Change(captured_piece.player, captured_piece.type, captured_piece.promoted, False, (to_col, to_row), None),
                                                Change(self.current_player, piece_type, promoted, True, (from_col, from_row), (to_col, to_row))
                                            ]
                                        )
                                break
                            # if the square is occupied by the same player break
                            else:
                                break

        # generate drop moves
        for piece_type, count in self.hand[self.current_player].items():
            if count > 0:
                min_row = 0
                max_row = 8
                # can't drop pawns, lances or knights on the last rank/ranks
                if piece_type in {PieceType.PAWN, PieceType.LANCE}:
                    if self.current_player == Player.BLACK:
                        min_row = 1
                    else:
                        max_row = 7
                elif piece_type == PieceType.KNIGHT:
                    if self.current_player == Player.BLACK:
                        min_row = 2
                    else:
                        max_row = 6
                # can't drop pawns on cols with other pawns
                cols = set(range(9))
                if piece_type == PieceType.PAWN:
                    for col, _ in self.piece_list[self.current_player][(PieceType.PAWN, False)]:
                        if col in cols:
                            cols.remove(col)
                # loop through the remaining rows and cols
                for row in range(min_row, max_row + 1):
                    for col in cols:
                        if self.board[col*9 + row] is None:
                            move =[Change(self.current_player, piece_type, False, False, None, (col, row))]
                            check_pawn_checkmate = False
                            # check if the pawn could checkmate the king
                            # this is only when the pawn is directly in front of the king, while protected
                            if piece_type == PieceType.PAWN:
                                _, delta_row = FIXED_MOVES_DICT[Piece(self.current_player, PieceType.PAWN, False)][0]
                                if self.board[col*9 + row + delta_row]==Piece(1-self.current_player, PieceType.KING, False):
                                    # check if the pawn is protected
                                    check_pawn_checkmate = self.is_square_attacked(self.current_player, (col, row))
                            # if the pawn could checkmate the king, make the move and explicitly check for checkmate
                            if check_pawn_checkmate:
                                game_state_copy = self.copy()
                                game_state_copy.move(move)
                                if not game_state_copy.is_game_over():
                                    moves.append(move)
                            else:
                                moves.append(move)
        return moves


    def is_square_attacked(self, player: Player, square: tuple[int, int]) -> bool:
        # generate inverse moves starting from the given square
        # and check if the piece is there
        col, row = square
        for piece_type, promoted in self.piece_list[player]:
            piece = Piece(player, piece_type, promoted)
            # check fixed moves first
            for delta_col, delta_row in FIXED_MOVES_DICT[piece]:
                from_col = col - delta_col
                from_row = row - delta_row
                if 0 <= from_col < 9 and 0 <= from_row < 9:
                    if self.board[from_col*9 + from_row] == piece:
                        return True
            # check sliding moves
            if piece in SLIDING_MOVES_DICT:
                for delta_col, delta_row in SLIDING_MOVES_DICT[piece]:
                    from_col = col - delta_col
                    from_row = row - delta_row
                    while 0 <= from_col < 9 and 0 <= from_row < 9:
                        if self.board[from_col*9 + from_row] == piece:
                            return True
                        elif self.board[from_col*9 + from_row] is not None:
                            break
                        from_col -= delta_col
                        from_row -= delta_row
        return False
    

    def is_check(self, player: Player) -> bool:
        # return cached result if present and it's not a temporary move
        if not self.temporary_move and self.check_cache[player] is not None:
            return self.check_cache[player]
        king_square = next(iter(self.piece_list[1-player][(PieceType.KING, False)]))
        result = self.is_square_attacked(player, king_square)
        # cache the result if not a temporary move
        if not self.temporary_move:
            self.check_cache[player] = result
        return result
    

    def is_square_attacked_by_sliding(self, player: Player, square: tuple[int, int], x_ray: bool = False) -> bool:
        col, row = square
        for piece_type, promoted in self.piece_list[player]:
            piece = Piece(player, piece_type, promoted)
            if piece in SLIDING_MOVES_DICT:
                for delta_col, delta_row in SLIDING_MOVES_DICT[piece]:
                    from_col = col - delta_col
                    from_row = row - delta_row
                    while 0 <= from_col < 9 and 0 <= from_row < 9:
                        if self.board[from_col*9 + from_row] == piece:
                            return True
                        # if the square is occupied by the same player we move on to the next direction
                        # unless x_ray is True: this allows checking for discovered checks more efficiently
                        elif self.board[from_col*9 + from_row] is not None and not x_ray:
                            break
                        from_col -= delta_col
                        from_row -= delta_row
        return False


    def move(self, move: List[Change]):
        self.move_temporary(move)
        self.temporary_move = False
        self.game_over = None
        self.winner = None
        self.legal_moves = None
        self.check_cache = {
            Player.BLACK: None,
            Player.WHITE: None,
        }
        self.move_count += 1


    def move_temporary(self, move: List[Change]):
        """
        Makes a move without updating the cached information.
        This is only used to make a move that will be undone soon.
        """
        for change in move:
            # remove piece from the old position
            if change.from_pos == None:
                self.hand[change.player][change.piece_type] -= 1
            else:
                c, r = change.from_pos
                self.board[c*9 + r] = None
                self.piece_list[change.player][(change.piece_type, change.from_promoted)].remove(change.from_pos)
            # add piece to the new position
            if change.to_pos == None:
                # the piece goes in the opponent's hand
                self.hand[1 - change.player][change.piece_type] += 1
            else:
                c, r = change.to_pos
                self.board[c*9 + r] = Piece(change.player, change.piece_type, change.to_promoted)
                self.piece_list[change.player][(change.piece_type, change.to_promoted)].add(change.to_pos)
        # switch player
        self.current_player = 1 - self.current_player
        
        # set temporary move flag
        # reset when unmoving
        self.temporary_move = True
    
    
    def unmove(self, move: List[Change]):
        self.unmove_temporary(move)
        self.game_over = None
        self.winner = None
        self.legal_moves = None
        self.check_cache = {
            Player.BLACK: None,
            Player.WHITE: None,
        }
        self.move_count -= 1


    def unmove_temporary(self, move: List[Change]):
        """
        Undoes a move without updating the cached information.
        This is only used to undo a move that was made temporarily.
        """
        for change in reversed(move):
            # remove piece from the new position
            if change.to_pos == None:
                # the piece went in the opponent's hand
                self.hand[1 - change.player][change.piece_type] -= 1
            else:
                c, r = change.to_pos
                self.board[c*9 + r] = None
                self.piece_list[change.player][(change.piece_type, change.to_promoted)].remove(change.to_pos)
            # add piece to the old position
            if change.from_pos == None:
                self.hand[change.player][change.piece_type] += 1
            else:
                c, r = change.from_pos
                self.board[c*9 + r] = Piece(change.player, change.piece_type, change.from_promoted)
                self.piece_list[change.player][(change.piece_type, change.from_promoted)].add(change.from_pos)
        # switch player
        self.current_player = 1 - self.current_player

        # reset temporary move flag
        self.temporary_move = False