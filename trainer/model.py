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

        self.ft = FeatureTransformer(NUM_FEATURES, ACCUMULATOR_SIZE)
        self.l1 = QuantLinear(ACCUMULATOR_SIZE * 2, 1, in_scale=127, out_scale=64)


    def forward(self, black_features, white_features, stm):
        # q_l1w = self.fake_quantize(self.ft.weight, 127, -32768, 32767)
        # q_l1b = self.fake_quantize(self.ft_bias, 127, -32768, 32767)
        # q_l2w = self.fake_quantize(self.l1.weight, 64, -128, 127)
        # q_l2b = self.fake_quantize(self.l1.bias, 64*127, torch.iinfo(torch.int32).min, torch.iinfo(torch.int32).max)
        # # no need to fake quantize the l2 bias at int32 precision, but usefoul to verify results


        b_acc = self.ft(black_features)
        w_acc = self.ft(white_features)

        accumulator = torch.where(
            stm.unsqueeze(-1) == 0,
            torch.cat([b_acc, w_acc], dim=1),
            torch.cat([w_acc, b_acc], dim=1)
        )

        accumulator = torch.clamp(accumulator, min=0., max=1.)
        return self.l1(accumulator)
    

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


def fake_quantize(x, scale, qmin, qmax):
    scale = 1/scale
    return torch.fake_quantize_per_tensor_affine(x, scale, 0, qmin, qmax)


class FeatureTransformer(nn.Module):
    def __init__(self, num_features, accumulator_size):
        super().__init__()
        self.weights = nn.Embedding(num_features, accumulator_size)
        self.biases = nn.Parameter(torch.zeros(accumulator_size))

        # initialize the weights closer to 0 to avoid too much saturation of the accumulator
        std = 1/np.sqrt(NUM_FEATURES)
        nn.init.normal_(self.weights.weight, mean=0., std=std)
        nn.init.normal_(self.biases, mean=0., std=std)

    @property
    def weight(self):
        return self.weights.weight
    
    @property
    def bias(self):
        return self.biases

    def forward(self, features):
        qt_w = fake_quantize(self.weights.weight, 127, -32768, 32767)
        qt_b = fake_quantize(self.biases, 127, -32768, 32767)
        return F.embedding(features, qt_w).sum(dim=1) + qt_b


class QuantLinear(nn.Module):
    def __init__(self, in_features, out_features, in_scale=127, out_scale=64):
        super().__init__()
        self.linear = nn.Linear(in_features, out_features)
        self.in_scale = in_scale
        self.out_scale = out_scale

    @property
    def weight(self):
        return self.linear.weight
    
    @property
    def bias(self):
        return self.linear.bias

    def forward(self, x):
        qt_w = fake_quantize(self.linear.weight, self.out_scale, -128, 127)
        qt_b = fake_quantize(self.linear.bias, self.in_scale*self.out_scale,
                             torch.iinfo(torch.int32).min, torch.iinfo(torch.int32).max)
        return F.linear(x, qt_w, qt_b)



if __name__ == "__main__":

    from nnue_loader import load_data_batch
    batch = load_data_batch("data/test_binps/0.binp")

    b_features = torch.tensor(batch.black_indexes)
    w_features = torch.tensor(batch.white_indexes)
    stm = torch.tensor(batch.stms)

    model = NNUEModel()

    # b_acc = model.ft(b_features).sum(dim=1) + model.ft_bias

    # print(b_acc.shape)
    # print(f'b_acc mean: {b_acc.mean().item():.4f}')
    # print(f'b_acc std:  {b_acc.std().item():.4f}')
    # print(f'b_acc fraction in (0,1):  {((b_acc > 0) & (b_acc < 1)).float().mean().item():.4f}')

    outs = model(b_features, w_features, stm)
    print(outs.shape)
    print(outs[0:10]*127*64)


    