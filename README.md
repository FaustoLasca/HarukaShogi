# Haruka Shogi Bot

In this repo I'm developing a shogi bot from scratch, for fun and to practice in these types of AI challenges.

## Get started

### 1 - Set up python env.
to create conda env with mamba run:
```bash
mamba env create -f environment.yml
```

### 2 - Install searchengine in the python env
activate the env:
```bash
mamba activate harukashogi
```
then install searchengine with the bash script:
```bash
bash install_searchengine.sh
```

## Updating the conda env
To update the environment.yml after installing a new dependency
```bash
mamba env export --no-builds > environment.yml
```
