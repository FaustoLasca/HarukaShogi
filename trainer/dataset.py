from pathlib import Path
import numpy as np
import torch
from torch.utils.data import IterableDataset, DataLoader, get_worker_info

from nnue_loader import load_data_batch


class NNUEIterableDataset(IterableDataset):
    def __init__(
        self, 
        data_path, 
        batch_size = 256, 
        shuffle = True, 
        random_hflip = False, 
        hflip = False
    ):
        self.files = list(Path(data_path).rglob("*.txt"))
        self.batch_size = batch_size
        self.shuffle = shuffle
        self.random_hflip = random_hflip
        self.hflip = hflip # always flip, overwritten by random_hflip

    def _files_fragment(self):
        info = get_worker_info()
        if info is None: return self.files
        # interleaving the files between workers
        else: return self.files[info.id::info.num_workers]


    def __iter__(self):
        files = self._files_fragment()

        if self.shuffle:
            np.random.shuffle(files)

        carry_b = carry_w = carry_s = carry_r = carry_t = None

        for file in files:
            batch = load_data_batch(
                file.as_posix(),
                random_hflip=self.random_hflip,
                hflip=self.hflip
            )
            b = torch.from_numpy(batch.black_indexes)
            w = torch.from_numpy(batch.white_indexes)
            s = torch.from_numpy(batch.scores)
            r = torch.from_numpy(batch.results)
            t = torch.from_numpy(batch.stms)

            if self.shuffle:
                perm = torch.randperm(b.size(0))
                b, w, s, r, t = b[perm], w[perm], s[perm], r[perm], t[perm]
            
            # concatenate the carry from the previous file
            if carry_b is not None:
                b = torch.cat([carry_b, b], dim=0)
                w = torch.cat([carry_w, w], dim=0)
                s = torch.cat([carry_s, s], dim=0)
                r = torch.cat([carry_r, r], dim=0)
                t = torch.cat([carry_t, t], dim=0)

            bs = self.batch_size
            limit = (b.size(0) // bs) * bs # if the batch size is bigger, skip loop
            for i in range(0, limit, bs):
                batch_b = b[i:i+bs].clone()
                batch_w = w[i:i+bs].clone()
                batch_s = s[i:i+bs].clone()
                batch_r = r[i:i+bs].clone()
                batch_t = t[i:i+bs].clone()
                yield (batch_b, batch_w, batch_s, batch_r, batch_t)
            
            carry_b = b[limit:].clone()
            carry_w = w[limit:].clone()
            carry_s = s[limit:].clone()
            carry_r = r[limit:].clone()
            carry_t = t[limit:].clone()
        
        # at the end, yield the remaining carry
        if carry_b is not None:
            if carry_b.size(0) > 0:
                yield (carry_b, carry_w, carry_s, carry_r, carry_t) 
                

if __name__ == "__main__":
    dataloader = DataLoader(
        NNUEIterableDataset("data/nnue/train", batch_size=10000),
        batch_size=None, # batch handled by the dataset
        num_workers=8,
        pin_memory=True,
        persistent_workers=True,
        prefetch_factor=2,
    )

    total = 0
    for batch in dataloader:
        b, w, s, r, t = batch
        print(b.shape, w.shape, s.shape, r.shape, t.shape)

        total += b.size(0)

    print(total)

    del dataloader
