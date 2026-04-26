import torch
from torch import nn
import torch.nn.functional as F
import numpy as np	

NUM_FEATURES = 2344
ACTIVE_FEATURES = 40
ACCUMULATOR_SIZE = 32

class NNUEModel(nn.Module):
    def __init__(self):
        super().__init__()

        self.ft = nn.Embedding(NUM_FEATURES, ACCUMULATOR_SIZE)
        self.ft_bias = nn.Parameter(torch.zeros(ACCUMULATOR_SIZE))
        self.l1 = nn.Linear(ACCUMULATOR_SIZE * 2, 1)

        std = 1/np.sqrt(NUM_FEATURES)
        nn.init.normal_(self.ft.weight, mean=0., std=std)
        nn.init.normal_(self.ft_bias, mean=0., std=std)


    @staticmethod
    def fake_quantize(x, scale, qmin, qmax):
        scale = 1/scale
        return torch.fake_quantize_per_tensor_affine(x, scale, 0, qmin, qmax)


    def forward(self, black_features, white_features, stm):
        q_l1w = self.fake_quantize(self.ft.weight, 127, -32768, 32767)
        q_l1b = self.fake_quantize(self.ft_bias, 127, -32768, 32767)
        q_l2w = self.fake_quantize(self.l1.weight, 64, -128, 127)
        # no need to fake quantize the l2 bias at int32 precision


        b_acc = F.embedding(black_features, q_l1w).sum(dim=1) + q_l1b
        w_acc = F.embedding(white_features, q_l1w).sum(dim=1) + q_l1b

        accumulator = torch.where(
            stm.unsqueeze(-1) == 0,
            torch.cat([b_acc, w_acc], dim=1),
            torch.cat([w_acc, b_acc], dim=1)
        )

        accumulator = torch.clamp(accumulator, min=0., max=1.)
        return F.linear(accumulator, q_l2w, self.l1.bias)
    

    def weights_to_bin(self, file_path):
        with open(file_path, "wb") as f:

            ft_wieghts = self.ft.weight.data.clone().cpu().numpy()           # (NUM_FEATURES, ACCUMULATOR_SIZE)
            ft_bias = self.ft_bias.data.clone().cpu().numpy()                # (ACCUMULATOR_SIZE,)
            l1_weights = self.l1.weight.data.clone().cpu().numpy().flatten() # (2*ACCUMULATOR_SIZE,)
            l1_bias = self.l1.bias.data.clone().cpu().numpy()                # (1,)

            ft_wieghts = (ft_wieghts * 127)                           .round().astype(np.int16)
            ft_bias    = (ft_bias    * 127)                           .round().astype(np.int16)
            l1_weights = (l1_weights * 64)    .clip(min=-128, max=127).round().astype(np.int8 )
            l1_bias    = (l1_bias    * 127*64)                        .round().astype(np.int32)

            f.write(ft_wieghts.tobytes())
            f.write(ft_bias.tobytes())
            f.write(l1_weights.tobytes())
            f.write(l1_bias.tobytes())


if __name__ == "__main__":

    from nnue_loader import load_data_batch
    batch = load_data_batch("data/gensfen/data_0/0.txt")

    b_features = torch.tensor(batch.black_indexes)
    w_features = torch.tensor(batch.white_indexes)
    stm = torch.tensor(batch.stms)

    model = NNUEModel()

    b_acc = model.ft(b_features).sum(dim=1) + model.ft_bias

    print(b_acc.shape)
    print(f'b_acc mean: {b_acc.mean().item():.4f}')
    print(f'b_acc std:  {b_acc.std().item():.4f}')
    print(f'b_acc fraction in (0,1):  {((b_acc > 0) & (b_acc < 1)).float().mean().item():.4f}')


    