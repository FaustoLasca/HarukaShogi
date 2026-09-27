import time
from datetime import timedelta
from pathlib import Path
import torch
import torch.nn as nn
from torch.optim import SGD, AdamW
from torch.optim.lr_scheduler import CosineAnnealingLR, StepLR, ExponentialLR
from torch.utils.data import DataLoader

from model import NNUEModel
from dataset import NNUEIterableDataset


device = "cuda"
LAMBDA = 0.8
EPOCHS = 50
CHECKPOINT_PATH = Path("trainer/checkpoint.pt")


def fmt_eta(sec: float) -> str:
    return str(timedelta(seconds=int(round(sec))))

def crossentropy_loss(output, target):
    eps = torch.finfo(output.dtype).eps
    loss = (target * torch.log(target + eps) + (1 - target) * torch.log(1 - target + eps)) \
         - (target * torch.log(output + eps) + (1 - target) * torch.log(1 - output + eps))
    return loss.mean()


def save_checkpoint(epoch):
    CHECKPOINT_PATH.parent.mkdir(parents=True, exist_ok=True)
    temporary_path = CHECKPOINT_PATH.with_suffix(".tmp")
    torch.save({
        "epoch": epoch,
        "model": model.state_dict(),
        "optimizer": optimizer.state_dict(),
        "scheduler": scheduler.state_dict(),
        "min_val_loss": min_val_loss,
    }, temporary_path)
    temporary_path.replace(CHECKPOINT_PATH)
    print(f"Saved checkpoint to {CHECKPOINT_PATH}")


model = NNUEModel(
    num_features=1696*45,
    num_buckets=45,
    accumulator_size=128,
    h1_size=8,
    h2_size=32,
).to(device)
model_name = "HalfKA_acc128-8-32_8B_08l.bin"

train_dataloader = DataLoader(
    NNUEIterableDataset("/home/fausto/myProjects/HarukaShogi/data/nnue/dataset_v2/train",
                        batch_size=16384*2, random_hflip=True, shuffle=True),
    batch_size=None,
    num_workers=16,
    persistent_workers=True,
    prefetch_factor=4,
    pin_memory=True,
)
val_dataloader = DataLoader(
    NNUEIterableDataset("/home/fausto/myProjects/HarukaShogi/data/nnue/dataset_v2/val",
                        batch_size=16384*2, random_hflip=False, shuffle=False),
    batch_size=None,
    num_workers=16,
    persistent_workers=True,
    prefetch_factor=4,
    pin_memory=True,
)
optimizer = AdamW(model.parameters(), lr=1e-4, weight_decay=1e-2)
scheduler = StepLR(optimizer, step_size=EPOCHS//5, gamma=0.5)

start_epoch = 0
min_val_loss = float('inf')
if CHECKPOINT_PATH.exists():
    checkpoint = torch.load(CHECKPOINT_PATH, map_location=device)
    model.load_state_dict(checkpoint["model"])
    optimizer.load_state_dict(checkpoint["optimizer"])
    scheduler.load_state_dict(checkpoint["scheduler"])
    start_epoch = checkpoint["epoch"]
    min_val_loss = checkpoint["min_val_loss"]
    print(f"Resuming from epoch {start_epoch}")

start_time = time.time()

train_losses = []
val_losses = []
val_res_losses = []
current_epoch = start_epoch
try:
    for epoch in range(start_epoch, EPOCHS):
        current_epoch = epoch
        for batch in train_dataloader:
            b, w, s, r, t = [tensor.to(device) for tensor in batch]

            s = s/(127*64)
            
            output = torch.sigmoid(model(b, w, t))
            target = (LAMBDA*torch.sigmoid(s*3.5) + (1 - LAMBDA)*r).unsqueeze(-1)

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
            target = (LAMBDA*torch.sigmoid(s*3.5) + (1 - LAMBDA)*r).unsqueeze(-1)

            loss = crossentropy_loss(output, target)
            val_losses.append(loss.detach())
            res_loss = crossentropy_loss(output, r.unsqueeze(-1))
            val_res_losses.append(res_loss.detach())

        elapsed_epochs = epoch - start_epoch + 1
        eta = (time.time() - start_time)/elapsed_epochs*(EPOCHS - epoch - 1)
        
        train_loss = torch.stack(train_losses).mean().item()
        val_loss = torch.stack(val_losses).mean().item()
        val_res_loss = torch.stack(val_res_losses).mean().item()
        print(f"Epoch {epoch}, \tLR: {scheduler.get_last_lr()[0]:.4e}, \t" \
              f"Train loss: {train_loss:.4f}, \tVal loss: {val_loss:.4f}, \t" \
              f"Val res loss: {val_res_loss:.4f}, \t" \
              f"ETA: {fmt_eta(eta)}")
        train_losses = []
        val_losses = []
        val_res_losses = []

        if val_loss < min_val_loss:
            min_val_loss = val_loss
            model.weights_to_bin(f"searchengine/bin/nnue/v2/{model_name}")
            print(f"Saved weights to {model_name}")

        scheduler.step()
        current_epoch = epoch + 1
        save_checkpoint(current_epoch)
except KeyboardInterrupt:
    save_checkpoint(current_epoch)
    print("Training interrupted")
finally:
    del train_dataloader
    del val_dataloader