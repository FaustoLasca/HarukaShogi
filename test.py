from search.alpha_beta.searcher import MinMaxSearcher
from search.evaluation.piece_value import PieceValueEvaluator
from search.evaluation.move_ordering import SimpleMoveOrderer
from game.game_state import GameState


evaluator = PieceValueEvaluator()
move_orderer = SimpleMoveOrderer()
searcher = MinMaxSearcher(GameState(), evaluator, move_orderer)

print(searcher.search(4))