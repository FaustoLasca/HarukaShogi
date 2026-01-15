import pygame
import sys

from game.game_state import GameState
from game.misc import Piece, Player, PieceType
from ui.misc import PIECE_IMAGE_PATHS

class GUI:
    def __init__(self, game_state: GameState) -> None:
        self.screen = pygame.display.set_mode((900, 900))
        pygame.display.set_caption("Haruka Shogi")
        self.clock = pygame.time.Clock()

        self.piece_images = dict[Piece, pygame.Surface]()
        for piece, path in PIECE_IMAGE_PATHS.items():
            self.piece_images[piece] = pygame.image.load(path).convert_alpha()
            self.piece_images[piece] = pygame.transform.scale(self.piece_images[piece], (100, 100))
            if piece.player == Player.BLACK:
                self.piece_images[piece] = pygame.transform.rotate(self.piece_images[piece], 180)
        self.board_tile_image = pygame.image.load("assets/board/tile_wood1.png")
        self.board_tile_image = pygame.transform.scale(self.board_tile_image, (98, 98))
        
        self.board = Board(game_state, self.piece_images, self.board_tile_image)

    def run(self):
        running = True
        while running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False
            self.screen.fill((0, 0, 0))
            self.board.draw(self.screen)
            pygame.display.flip()
            self.clock.tick(60)
        pygame.quit()
        sys.exit()


class GUIElement:
    def draw(self, screen):
        pass


class PieceGUIElement(GUIElement):
    def __init__(
        self,
        image: pygame.Surface,
        cell: tuple[int, int],
        board_start_pos: tuple[int, int] = (0, 0),
        cell_size: int = 100
    ) -> None:
        self.image = image
        self.cell_col, self.cell_row = cell
        self.size = cell_size
        self.x = (8.5 - self.cell_col) * cell_size + board_start_pos[0]
        self.y = (0.5 + self.cell_row) * cell_size + board_start_pos[1]
        self.rect = image.get_rect(center=(self.x, self.y))

    def draw(self, screen):
        screen.blit(self.image, self.rect)


class Board(GUIElement):
    def __init__(
        self,
        game_state: GameState,
        piece_images: dict[Piece, pygame.Surface],
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

    def initialize_pieces(self, piece_list: dict[Piece, set[tuple[int, int]]]):
        pieces = []
        for piece, cells in piece_list.items():
            for cell in cells:
                pieces.append(PieceGUIElement(
                    image=self.piece_images[piece],
                    cell=cell,
                    board_start_pos=self.board_start_pos,
                    cell_size=self.cell_size,
                ))
        return pieces

    def draw(self, screen):
        for i in range(9):
            for j in range(9):
                screen.blit(self.board_tile_image, (i*self.cell_size, j*self.cell_size))
        for piece in self.pieces:
            piece.draw(screen)
