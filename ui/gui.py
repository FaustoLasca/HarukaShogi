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
        self.board = Board(self.piece_images, self.board_tile_image, (450, 0))
        self.hands = {
            Player.WHITE: Hand(Player.WHITE, self.piece_images, self.hand_image, (0, 0), 400),
            Player.BLACK: Hand(Player.BLACK, self.piece_images, self.hand_image, (1400, 500), 400),
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
            try:
                move, state = self.update_queue.get_nowait()
                self.update(move, state)
            except Empty:
                pass
            try:
                available_moves = self.move_response_queue.get_nowait()
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
        pos: tuple[int, int]
    ) -> None:
        self.image = image
        self.x, self.y = pos
        self.rect = image.get_rect(center=(self.x, self.y))

    def draw(self, screen):
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
        piece_images: dict[PieceClass, pygame.Surface],
        board_tile_image: pygame.Surface,
        board_start_pos: tuple[int, int] = (0, 0),
        board_size: int = 900,
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


    def initialize_pieces(self, piece_list: dict[PieceClass, set[tuple[int, int]]]):
        self.pieces = []
        for player in Player:
            for tuple, cells in piece_list[player].items():
                piece_type, promoted = tuple
                for col, row in cells:
                    x = (8.5 - col) * self.cell_size + self.board_start_pos[0]
                    y = (0.5 + row) * self.cell_size + self.board_start_pos[1]
                    self.pieces.append(Piece(
                        image=self.piece_images[PieceClass(player, piece_type, promoted)],
                        pos=(x, y)
                    ))


    def highlight_last_move(self, move: List[Change]):
        self.move_highlights = []
        positions = [move[-1].from_pos, move[-1].to_pos]
        for pos in positions:
            if pos is not None:
                x, y = self.pos_to_pixel(pos, mode="topleft")
                self.move_highlights.append(HighlightSquare(
                    color=(255, 255, 0, 100),
                    pos=(x, y), size=self.cell_size)
                )

    
    def update(self, move: List[Change], state: GameState):
        self.initialize_pieces(state.piece_list)
        if move is not None:
            self.highlight_last_move(move)


    def draw(self, screen):
        # draw board tiles
        for i in range(9):
            for j in range(9):
                rect = self.board_tile_image.get_rect(center=((i + 0.5)*self.cell_size + self.board_start_pos[0], (j + 0.5)*self.cell_size + self.board_start_pos[1]))
                screen.blit(self.board_tile_image, rect)
        # draw move highlights
        for highlight in self.move_highlights:
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
        

class Hand(GUIElement):
    def __init__(
        self,
        player: Player,
        piece_images: dict[PieceClass, pygame.Surface],
        hand_image: pygame.Surface,
        start_pos: tuple[int, int],
        size: int = 400,
    ) -> None:
        self.player = player
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

    def initialize_pieces(self, hand: dict[PieceType, int]):
        self.pieces = []
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
                    pos=(x, y)
                ))
                n_pieces += 1
    
    def update(self, move: List[Change], state: GameState):
        self.initialize_pieces(state.hand[self.player])

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