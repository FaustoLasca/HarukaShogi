import pygame
import sys
from queue import Queue, Empty
from typing import List

from game.game_state import GameState
from game.misc import Player, PieceType, Change
from game.misc import Piece as PieceClass
from ui.misc import PIECE_IMAGE_PATHS

class Gui:
    def __init__(
        self,
        update_queue: Queue[(List[Change], GameState)],
        move_request_queue: Queue[List[Change]],
        move_response_queue: Queue[List[Change]]
    ) -> None:
        pygame.init()
        # Explicitly disallow window resizing by passing flags=0 (no RESIZABLE flag)
        self.screen = pygame.display.set_mode((900 + 800 + 100, 900), flags=0)
        pygame.display.set_caption("Haruka Shogi")
        self.clock = pygame.time.Clock()

        # loadimages
        self.piece_images = dict[PieceClass, pygame.Surface]()
        for piece, path in PIECE_IMAGE_PATHS.items():
            self.piece_images[piece] = pygame.image.load(path).convert_alpha()
            self.piece_images[piece] = pygame.transform.scale(self.piece_images[piece], (100, 100))
            if piece.player == Player.BLACK:
                self.piece_images[piece] = pygame.transform.rotate(self.piece_images[piece], 180)
        self.board_tile_image = pygame.image.load("assets/board/tile_wood1.png")
        self.board_tile_image = pygame.transform.scale(self.board_tile_image, (98, 98))
        self.hand_image = pygame.image.load("assets/board/tile_wood1.png")
        self.hand_image = pygame.transform.scale(self.hand_image, (400, 400))

        # queues to communicate with controller
        self.update_queue = update_queue
        self.move_request_queue = move_request_queue
        self.move_response_queue = move_response_queue
        
        # initialize elements
        self.board = Board(self.move_response_queue, self.piece_images, self.board_tile_image, (450, 0))
        self.hands = {
            Player.WHITE: Hand(Player.WHITE, self.move_response_queue, self.board, self.piece_images, self.hand_image, (0, 0), 400),
            Player.BLACK: Hand(Player.BLACK, self.move_response_queue, self.board, self.piece_images, self.hand_image, (1400, 500), 400),
        }
        self.winning_message = WinningMessage((1600, 100), 80)
    
    def update(self, move: List[Change], state: GameState):
        self.board.update(move, state)
        for player in Player:
            self.hands[player].update(move, state)
        state.generate_moves()
        if state.game_over:
            self.winning_message.update(state.winner)

    def run(self):
        running = True
        while running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False
                # mouse interaction for moves
                if event.type == pygame.MOUSEBUTTONDOWN:
                    self.board.handle_mouse_press(event.pos)
                    for player in Player:
                        self.hands[player].handle_mouse_press(event.pos)
                if event.type == pygame.MOUSEBUTTONUP:
                    self.board.handle_mouse_release(event.pos, event.button)
                    for player in Player:
                        self.hands[player].handle_mouse_release(event.pos)
            try:
                move, state = self.update_queue.get_nowait()
                self.update(move, state)
            except Empty:
                pass
            try:
                available_moves = self.move_request_queue.get_nowait()
                self.board.update_available_moves(available_moves)
                for player in Player:
                    self.hands[player].update_available_moves(available_moves)
            except Empty:
                pass
            self.screen.fill((0, 0, 0))
            self.board.draw(self.screen)
            for player in Player:
                self.hands[player].draw(self.screen)
            self.winning_message.draw(self.screen)
            pygame.display.flip()
            self.clock.tick(60)
        pygame.quit()
        sys.exit()


class GUIElement:
    def draw(self, screen):
        pass


class Piece(GUIElement):
    def __init__(
        self,
        image: pygame.Surface,
        pos: tuple[int, int],
        piece_type: PieceType,
        board_pos: tuple[int, int],
    ) -> None:
        self.image = image
        self.x, self.y = pos
        self.rect = image.get_rect(center=(self.x, self.y))
        self.piece_type = piece_type
        self.board_pos = board_pos
        self.is_moving = False

    def mouse_collides(self, pixel_pos: tuple[int, int]) -> bool:
        return self.rect.collidepoint(pixel_pos)

    def start_moving(self):
        self.is_moving = True

    def stop_moving(self):
        self.is_moving = False
        self.rect.center = (self.x, self.y)

    def draw(self, screen):
        if self.is_moving:
            self.rect.center = pygame.mouse.get_pos()
        screen.blit(self.image, self.rect)


class HighlightSquare(GUIElement):
    def __init__(
        self,
        color: tuple[int, int, int, int],
        pos: tuple[int, int],
        size: int
    ) -> None:
        self.color = color
        self.x, self.y = pos
        self.size = size
        self.surface = pygame.Surface((self.size, self.size), pygame.SRCALPHA)
        self.surface.fill(self.color)
        self.rect = pygame.Rect(self.x, self.y, self.size, self.size)

    def draw(self, screen):
        screen.blit(self.surface, self.rect)


class Board(GUIElement):
    def __init__(
        self,
        move_response_queue: Queue[List[Change]],
        piece_images: dict[PieceClass, pygame.Surface],
        board_tile_image: pygame.Surface,
        board_start_pos: tuple[int, int] = (0, 0),
        board_size: int = 900
    ) -> None:
        self.board_tile_image = board_tile_image
        self.piece_images = piece_images
        self.board_start_pos = board_start_pos
        self.cell_size = board_size / 9
        empty_piece_list = {
            Player.WHITE: {},
            Player.BLACK: {},
        }
        self.initialize_pieces(empty_piece_list)
        self.move_highlights = []

        self.move_response_queue = move_response_queue
        self.can_move = False
        self.available_moves = {}
        self.moving_piece : Piece= None
        self.available_highlights : dict[tuple[int, int], HighlightSquare] = {}


    def initialize_pieces(self, piece_list: dict[PieceClass, set[tuple[int, int]]]):
        self.pieces: List[Piece] = []
        for player in Player:
            for tuple, cells in piece_list[player].items():
                piece_type, promoted = tuple
                for col, row in cells:
                    x = (8.5 - col) * self.cell_size + self.board_start_pos[0]
                    y = (0.5 + row) * self.cell_size + self.board_start_pos[1]
                    self.pieces.append(Piece(
                        image=self.piece_images[PieceClass(player, piece_type, promoted)],
                        pos=(x, y),
                        piece_type=piece_type,
                        board_pos=(col, row),
                    ))


    def highlight_last_move(self, move: List[Change]):
        self.move_highlights = []
        positions = [move[-1].from_pos, move[-1].to_pos]
        for pos in positions:
            if pos is not None:
                x, y = self.pos_to_pixel(pos, mode="topleft")
                self.move_highlights.append(HighlightSquare(
                    color=(255, 255, 0, 120),
                    pos=(x+1, y+1), size=self.cell_size-2)
                )

    
    def update(self, move: List[Change], state: GameState):
        self.initialize_pieces(state.piece_list)
        if move is not None:
            self.highlight_last_move(move)


    def update_available_moves(self, available_moves: List[List[Change]]):
        self.available_moves: dict[tuple[int, int], List[List[Change]]] = {}
        for move in available_moves:
            if move[-1].from_pos is not None:
                if move[-1].from_pos not in self.available_moves:
                    self.available_moves[move[-1].from_pos] = []
                self.available_moves[move[-1].from_pos].append(move)
        self.can_move = True


    def handle_mouse_press(self, pixel_pos: tuple[int, int]):
        if self.can_move:
            for piece in self.pieces:
                if piece.mouse_collides(pixel_pos):
                    start_pos = piece.board_pos
                    if start_pos in self.available_moves:
                        self.moving_piece = piece
                        piece.start_moving()
                        for move in self.available_moves[start_pos]:
                            col, row = move[-1].to_pos
                            if (col, row) not in self.available_highlights:
                                x = (8 - col) * self.cell_size + self.board_start_pos[0]
                                y = (row) * self.cell_size + self.board_start_pos[1]
                                self.available_highlights[move[-1].to_pos] = HighlightSquare(
                                    color=(255, 0, 0, 100),
                                    pos=(x+1, y+1),
                                    size=self.cell_size-2
                                )


    def handle_mouse_release(self, pixel_pos: tuple[int, int], button: int):
        if self.moving_piece is not None:
            end_pos = self.pixel_to_pos(pixel_pos)
            chosen_moves = []

            for move in self.available_moves[self.moving_piece.board_pos]:
                if move[-1].to_pos == end_pos:
                    chosen_moves.append(move)

            if len(chosen_moves) == 1:
                self.move_response_queue.put(chosen_moves[0])
                self.can_move = False

            # this happens when there is a promotion choice
            elif len(chosen_moves) > 1:
                if button == 1:
                    self.move_response_queue.put(chosen_moves[0] if not chosen_moves[0][-1].to_promoted else chosen_moves[1])
                elif button == 3:
                    self.move_response_queue.put(chosen_moves[0] if chosen_moves[0][-1].to_promoted else chosen_moves[1])
                self.can_move = False

            self.moving_piece.stop_moving()
            self.moving_piece = None
            self.available_highlights = {}


    def draw(self, screen):
        # draw board tiles
        for i in range(9):
            for j in range(9):
                rect = self.board_tile_image.get_rect(center=((i + 0.5)*self.cell_size + self.board_start_pos[0], (j + 0.5)*self.cell_size + self.board_start_pos[1]))
                screen.blit(self.board_tile_image, rect)
        # draw move highlights
        for highlight in self.move_highlights:
            highlight.draw(screen)
        for highlight in self.available_highlights.values():
            highlight.draw(screen)
        # draw pieces
        for piece in self.pieces:
            piece.draw(screen)


    def pos_to_pixel(self, pos: tuple[int, int], mode: str = "topleft") -> tuple[int, int]:
        col, row = pos
        x = (8 - col) * self.cell_size + self.board_start_pos[0]
        y = (row) * self.cell_size + self.board_start_pos[1]
        if mode == "topleft":
            return (x, y)
        elif mode == "center":
            return (x + self.cell_size / 2, y + self.cell_size / 2)
        else:
            raise ValueError(f"Invalid mode: {mode}")

    def pixel_to_pos(self, pixel_pos: tuple[int, int]) -> tuple[int, int]:
        x, y = pixel_pos
        col = 8 -int((x - self.board_start_pos[0]) / self.cell_size)
        row = int((y - self.board_start_pos[1]) / self.cell_size)
        return (col, row)
        

class Hand(GUIElement):
    def __init__(
        self,
        player: Player,
        move_response_queue: Queue[List[Change]],
        board: Board,
        piece_images: dict[PieceClass, pygame.Surface],
        hand_image: pygame.Surface,
        start_pos: tuple[int, int],
        size: int = 400,
    ) -> None:
        self.player = player
        self.board = board
        self.piece_images = piece_images
        self.hand_image = hand_image
        self.start_pos = start_pos
        self.size = size
        self.n_col = 4
        self.n_row = 4
        self.col_size = self.size / self.n_col
        self.row_size = self.size / self.n_row
        empty_hand = {}
        self.initialize_pieces(empty_hand)

        self.move_response_queue = move_response_queue
        self.can_move = False
        self.moving_piece : Piece = None
        self.available_moves : dict[PieceType, List[List[Change]]] = {}

    def initialize_pieces(self, hand: dict[PieceType, int]):
        self.pieces: List[Piece] = []
        n_pieces = 0
        for piece_type, count in hand.items():
            for i in range(count):
                if n_pieces > self.n_col * self.n_row:
                    break
                col = n_pieces % self.n_col
                row = n_pieces // self.n_col
                if self.player == Player.WHITE:
                    col = self.n_col - col - 1
                    row = self.n_row - row - 1
                x = (col + 0.5) * self.col_size + self.start_pos[0]
                y = (row + 0.5) * self.row_size + self.start_pos[1]
                self.pieces.append(Piece(
                    image=self.piece_images[PieceClass(self.player, piece_type, False)],
                    pos=(x, y),
                    piece_type=piece_type,
                    board_pos=None,
                ))
                n_pieces += 1
    
    def update(self, move: List[Change], state: GameState):
        self.initialize_pieces(state.hand[self.player])


    def update_available_moves(self, available_moves: List[List[Change]]):
        self.available_moves: dict[PieceType, List[List[Change]]] = {}
        for move in available_moves:
            if move[-1].from_pos is None:
                for piece in self.pieces:
                    if piece.piece_type == move[-1].piece_type:
                        if piece.piece_type not in self.available_moves:
                            self.available_moves[piece.piece_type] = []
                        self.available_moves[piece.piece_type].append(move)
        self.can_move = True


    def handle_mouse_press(self, pixel_pos: tuple[int, int]):
        if self.can_move:
            for piece in self.pieces:
                if piece.mouse_collides(pixel_pos):
                    piece_type = piece.piece_type
                    if piece_type in self.available_moves:
                        self.moving_piece = piece
                        piece.start_moving()


    def handle_mouse_release(self, pixel_pos: tuple[int, int]):
        if self.moving_piece is not None:
            end_pos = self.board.pixel_to_pos(pixel_pos)
            chosen_move = None

            for move in self.available_moves[self.moving_piece.piece_type]:
                if move[-1].to_pos == end_pos:
                    chosen_move = move
                    break

            if chosen_move is not None:
                self.move_response_queue.put(chosen_move)
                self.can_move = False

            self.moving_piece.stop_moving()
            self.moving_piece = None


    def draw(self, screen):
        # rect = self.hand_image.get_rect(topleft=self.start_pos)
        # screen.blit(self.hand_image, rect)
        for piece in self.pieces:
            piece.draw(screen)


class WinningMessage(GUIElement):
    def __init__(
        self,
        pos: tuple[int, int],
        size: int
    ) -> None:
        self.x, self.y = pos
        self.font = pygame.font.Font(None, size)
        self.size = size
        self.messages = []
        self.rects = []

    def update(self, winner: Player):
        message1 = "Game Over!"
        if winner == Player.BLACK:
            message2 = " Black wins!"
        elif winner == Player.WHITE:
            message2 = " White wins!"
        else:
            message2 = " Draw!"
        self.messages = [
            self.font.render(message1, True, (255, 255, 255)),
            self.font.render(message2, True, (255, 255, 255))
            ]
        self.rects = [
            self.messages[0].get_rect(center=(self.x, self.y)),
            self.messages[1].get_rect(center=(self.x, self.y + self.size))
        ]

    def draw(self, screen):
        for message, rect in zip(self.messages, self.rects):
            screen.blit(message, rect)