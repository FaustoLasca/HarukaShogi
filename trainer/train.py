import torch
import torch.nn as nn
from torch.optim import SGD, AdamW
from torch.optim.lr_scheduler import CosineAnnealingLR, StepLR, ExponentialLR
from torch.utils.data import DataLoader

from model import NNUEModel
from dataset import NNUEIterableDataset


device = "cuda"
LAMBDA = 0.7
EPOCHS = 300


model = NNUEModel().to(device)
dataloader = DataLoader(
    NNUEIterableDataset("data/nnue/train", batch_size=16384, random_hflip=True),
    batch_size=None,
    num_workers=16,
    persistent_workers=True,
    prefetch_factor=4,
    pin_memory=True,
    )
optimizer = AdamW(model.parameters(), lr=1e-3, weight_decay=1e-2)
scheduler = ExponentialLR(optimizer, gamma=0.1**(1/EPOCHS))

losses = []
for epoch in range(EPOCHS):
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

        losses.append(loss.detach())
    
    mean_loss = torch.stack(losses).mean().item()
    print(f"Epoch {epoch}, Loss: {mean_loss:.4f}, LR: {scheduler.get_last_lr()[0]:.4e}")
    losses = []

    scheduler.step()

model.weights_to_bin("searchengine/bin/nnue/AdamW_acc8_14M_1wd.bin")

del dataloader	