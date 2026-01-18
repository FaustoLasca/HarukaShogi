from search.alpha_beta.searcher import MinMaxSearcher
from search.evaluation.piece_value import SimpleEvaluator
from search.evaluation.move_ordering import SimpleMoveOrderer
from game.game_state import GameState
import random


evaluator = SimpleEvaluator()
move_orderer = SimpleMoveOrderer()
searcher = MinMaxSearcher(GameState(), evaluator)

searcher.search(3)