import numpy as np
from nnue_loader import load_data_batch

if __name__ == "__main__":
    batch = load_data_batch("test_data.txt")

    print(type(batch.batch_size))
    print(type(batch.black_indexes))
    print(type(batch.white_indexes))
    print(type(batch.scores))
    print(type(batch.results))
    print(type(batch.stms))

    print(batch.batch_size)
    print(batch.black_indexes)
    print(batch.white_indexes)
    print(batch.scores)
    print(batch.results)
    print(batch.stms)
