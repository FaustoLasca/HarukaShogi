import torch
from torch import nn
import numpy as np	

NUM_FEATURES = 2344
ACTIVE_FEATURES = 40
ACCUMULATOR_SIZE = 8

class NNUEModel(nn.Module):
    def __init__(self):
        super().__init__()

        self.l1 = nn.Embedding(NUM_FEATURES, ACCUMULATOR_SIZE)
        self.l1_bias = nn.Parameter(torch.zeros(ACCUMULATOR_SIZE))
        self.l2 = nn.Linear(ACCUMULATOR_SIZE * 2, 1)

    def forward(self, black_features, white_features, stm):
        b_acc = self.l1(black_features).sum(dim=1) + self.l1_bias
        w_acc = self.l1(white_features).sum(dim=1) + self.l1_bias

        accumulator = torch.where(
            stm.unsqueeze(-1) == 0,
            torch.cat([b_acc, w_acc], dim=1),
            torch.cat([w_acc, b_acc], dim=1)
        )

        accumulator = torch.clamp(accumulator, min=0., max=1.)
        return self.l2(accumulator)

    def weights_to_bin(self, file_path):
        with open(file_path, "wb") as f:

            l1_weights = self.l1.weight.data.numpy()           # (NUM_FEATURES, ACCUMULATOR_SIZE)
            l1_bias = self.l1_bias.data.numpy()                # (ACCUMULATOR_SIZE,)
            l2_weights = self.l2.weight.data.numpy().flatten() # (2*ACCUMULATOR_SIZE,)
            l2_bias = self.l2.bias.data.numpy()                # (1,)

            l1_weights = (l1_weights * 127).round().astype(np.int16)
            l1_bias    = (l1_bias * 127)   .round().astype(np.int16)
            l2_weights = (l2_weights * 64) .round().astype(np.int8)
            l2_bias    = (l2_bias * 127*64).round().astype(np.int32)

            print(l1_weights.shape, l1_weights.dtype)
            print(l1_bias.shape, l1_bias.dtype)
            print(l2_weights.shape, l2_weights.dtype)
            print(l2_bias.shape, l2_bias.dtype)

            f.write(l1_weights.tobytes())
            f.write(l1_bias.tobytes())
            f.write(l2_weights.tobytes())
            f.write(l2_bias.tobytes())


if __name__ == "__main__":

    from nnue_loader import load_data_batch
    batch = load_data_batch("data/gensfen/data_0/0.txt")

    b_features = torch.tensor(batch.black_indexes)
    w_features = torch.tensor(batch.white_indexes)
    stm = torch.tensor(batch.stms)

    print(b_features.shape, w_features.shape, stm.unsqueeze(-1).shape)
    print(stm.unsqueeze(-1))

    model = NNUEModel()
    model.eval()
    with torch.no_grad():
        output = model(b_features, w_features, stm)
    print(output * 600)

    model.weights_to_bin("searchengine/bin/nnue/test_weights.bin")