import torch
import torch.nn as nn
from torch.optim import SGD
from torch.utils.data import DataLoader

from model import NNUEModel
from dataset import NNUEIterableDataset


device = "cuda"
LAMBDA = 0.7


model = NNUEModel().to(device)
dataloader = DataLoader(
    NNUEIterableDataset("data/gensfen", batch_size=8192),
    batch_size=None,
    num_workers=16,
    persistent_workers=True,
    prefetch_factor=4,
    pin_memory=True,
    )
optimizer = SGD(model.parameters(), lr=1e-2, weight_decay=1e-5)

losses = []
for epoch in range(300):
    for batch in dataloader:
        b, w, s, r, t = [tensor.to(device) for tensor in batch]

        s = s/(127*64)
        
        output = torch.sigmoid(model(b, w, t))
        target = (LAMBDA*torch.sigmoid(s*4) + (1 - LAMBDA)*r).unsqueeze(-1)

        eps = torch.finfo(output.dtype).eps
        loss = (target * torch.log(target + eps) + (1 - target) * torch.log(1 - target + eps)) \
             - (target * torch.log(output + eps) + (1 - target) * torch.log(1 - output + eps))
        loss = loss.mean()

        optimizer.zero_grad()
        loss.backward()
        optimizer.step()

        losses.append(loss)

    mean_loss = torch.stack(losses).mean().item()
    print(f"Epoch {epoch}, Loss: {mean_loss:.4f}")
    losses = []

model.weights_to_bin("searchengine/bin/nnue/weights_3.bin")

del dataloader	