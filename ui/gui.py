from numpy.char import center
import pygame
import sys

from game.game_state import GameState
from game.misc import Player, PieceType
from game.misc import Piece as PieceClass
from ui.misc import PIECE_IMAGE_PATHS

class GUI:
    def __init__(self, game_state: GameState) -> None:
        # Explicitly disallow window resizing by passing flags=0 (no RESIZABLE flag)
        self.screen = pygame.display.set_mode((900 + 800 + 100, 900), flags=0)
        pygame.display.set_caption("Haruka Shogi")
        self.clock = pygame.time.Clock()

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
        
        self.board = Board(game_state, self.piece_images, self.board_tile_image, (450, 0))
        self.hands = {
            Player.WHITE: Hand(Player.WHITE, game_state, self.piece_images, self.hand_image, (0, 0), 400),
            Player.BLACK: Hand(Player.BLACK, game_state, self.piece_images, self.hand_image, (1400, 500), 400),
        }

    def run(self):
        running = True
        while running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False
            self.screen.fill((0, 0, 0))
            self.board.draw(self.screen)
            for player in Player:
                self.hands[player].draw(self.screen)
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


class Board(GUIElement):
    def __init__(
        self,
        game_state: GameState,
        piece_images: dict[PieceClass, pygame.Surface],
        board_tile_image: pygame.Surface,
        board_start_pos: tuple[int, int] = (0, 0),
        board_size: int = 900,
    ) -> None:
        self.board_tile_image = board_tile_image
        self.piece_images = piece_images
        self.piece_list = game_state.piece_list
        self.board_start_pos = board_start_pos
        self.cell_size = board_size / 9
        self.pieces = self.initialize_pieces(self.piece_list)

    def initialize_pieces(self, piece_list: dict[PieceClass, set[tuple[int, int]]]):
        pieces = []
        for player in Player:
            for tuple, cells in self.piece_list[player].items():
                piece_type, promoted = tuple
                for col, row in cells:
                    x = (8.5 - col) * self.cell_size + self.board_start_pos[0]
                    y = (0.5 + row) * self.cell_size + self.board_start_pos[1]
                    pieces.append(Piece(
                        image=self.piece_images[PieceClass(player, piece_type, promoted)],
                        pos=(x, y)
                    ))
        return pieces

    def draw(self, screen):
        for i in range(9):
            for j in range(9):
                rect = self.board_tile_image.get_rect(center=((i + 0.5)*self.cell_size + self.board_start_pos[0], (j + 0.5)*self.cell_size + self.board_start_pos[1]))
                screen.blit(self.board_tile_image, rect)
        for piece in self.pieces:
            piece.draw(screen)


class Hand(GUIElement):
    def __init__(
        self,
        player: Player,
        game_state: GameState,
        piece_images: dict[PieceClass, pygame.Surface],
        hand_image: pygame.Surface,
        start_pos: tuple[int, int],
        size: int = 400,
    ) -> None:
        self.player = player
        self.hand = game_state.hand[player]
        self.piece_images = piece_images
        self.hand_image = hand_image
        self.start_pos = start_pos
        self.size = size
        self.n_col = 4
        self.n_row = 4
        self.col_size = self.size / self.n_col
        self.row_size = self.size / self.n_row
        self.pieces = self.initialize_pieces(self.hand)

    def initialize_pieces(self, hand: dict[PieceType, int]):
        pieces = []
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
                pieces.append(Piece(
                    image=self.piece_images[PieceClass(self.player, piece_type, False)],
                    pos=(x, y)
                ))
                n_pieces += 1
        return pieces
    
    def draw(self, screen):
        # rect = self.hand_image.get_rect(topleft=self.start_pos)
        # screen.blit(self.hand_image, rect)
        for piece in self.pieces:
            piece.draw(screen)