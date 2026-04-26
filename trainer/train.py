import torch
import torch.nn as nn
from torch.optim import SGD, AdamW
from torch.optim.lr_scheduler import CosineAnnealingLR, StepLR, ExponentialLR
from torch.utils.data import DataLoader

from model import NNUEModel
from dataset import NNUEIterableDataset


device = "cuda"
LAMBDA = 0.7
EPOCHS = 200


def crossentropy_loss(output, target):
    eps = torch.finfo(output.dtype).eps
    loss = (target * torch.log(target + eps) + (1 - target) * torch.log(1 - target + eps)) \
         - (target * torch.log(output + eps) + (1 - target) * torch.log(1 - output + eps))
    return loss.mean()


model = NNUEModel().to(device)
train_dataloader = DataLoader(
    NNUEIterableDataset("data/nnue/train", batch_size=16384, random_hflip=True),
    batch_size=None,
    num_workers=16,
    persistent_workers=True,
    prefetch_factor=4,
    pin_memory=True,
)
val_dataloader = DataLoader(
    NNUEIterableDataset("data/nnue/val", batch_size=16384, random_hflip=False, shuffle=False),
    batch_size=None,
    num_workers=16,
    persistent_workers=True,
    prefetch_factor=4,
    pin_memory=True,
)
optimizer = AdamW(model.parameters(), lr=1e-3, weight_decay=1e-2)
scheduler = ExponentialLR(optimizer, gamma=0.1**(1/EPOCHS))

train_losses = []
val_losses = []
min_val_loss = float('inf')
for epoch in range(EPOCHS):
    for batch in train_dataloader:
        b, w, s, r, t = [tensor.to(device) for tensor in batch]

        s = s/(127*64)
        
        output = torch.sigmoid(model(b, w, t))
        target = (LAMBDA*torch.sigmoid(s*4) + (1 - LAMBDA)*r).unsqueeze(-1)

        loss = crossentropy_loss(output, target)

        optimizer.zero_grad()
        loss.backward()
        optimizer.step()

        train_losses.append(loss.detach())

    
    for batch in val_dataloader:
        b, w, s, r, t = [tensor.to(device) for tensor in batch]

        s = s/(127*64)
        
        with torch.no_grad():
            output = torch.sigmoid(model(b, w, t))
        target = (LAMBDA*torch.sigmoid(s*4) + (1 - LAMBDA)*r).unsqueeze(-1)

        loss = crossentropy_loss(output, target)
        val_losses.append(loss.detach())
    
    train_loss = torch.stack(train_losses).mean().item()
    val_loss = torch.stack(val_losses).mean().item()
    print(f"Epoch {epoch}, \tLR: {scheduler.get_last_lr()[0]:.4e}, \t" \
          f"Train loss: {train_loss:.4f}, \tVal loss: {val_loss:.4f}")
    train_losses = []
    val_losses = []

    if val_loss < min_val_loss:
        min_val_loss = val_loss
        model.weights_to_bin("searchengine/bin/nnue/AdamW_acc32_30M.bin")
        print("Model saved to searchengine/bin/nnue/AdamW_acc32_30M.bin")


    scheduler.step()

del train_dataloader
del val_dataloader