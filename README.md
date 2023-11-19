# parallelMatrixMultiplication

## Instructions

### Install required python libraries

```bash
pip install numpy joblib pandas seaborn
```

### Compile using make

```bash
make
```

### Run timing experiments

python3 timing_experiments.py

You can setup N_REPETITIONS in the code. This will configure how many times a single file of multiplication pairs is run in a given configuration (in example coarse grained with 4 threads).

The idea is to estimate the timing from more than one measure of the execution time.

### Running the analysis

To collect the results and produce the plots.

```bash
python3 analysis.py results/
```

The plots will generate in the results/plots folder.

### Running individual codes

There are four ways to run the multiplications:

1. C-based "REF" (the one provided by the course)
1. C-based "COARSE" (runs a single multiplication of pairs per thread until there are none left)
1. C-based "FINE" (runs 2 row*column products of the 2 matrices per thread)
1. C-based "FINEHUNGRY" (runs 2 row*column products of the 2 matrices per thread using more memory)
1. "PYTHON" (Python parallelism of the coarse type + C function)

For C-based multiplication use

```bash
./matmulseq_file N_THREADS FILE MODE VERBOSE
```

E.g.

```bash
./matmulseq_file 4 matrices_large.dat FINE 0
```

For Python+C multiplication use:

```bash
python3 matmulseq_file.py /path/to/matrix/file --n_jobs 4
```

E.g.

```bash
/usr/bin/python3 /home/user/code/parallelMatrixMultiplication/matmulseq_file.py /home/user/code/parallelMatrixMultiplication/matrices_large.dat --n_jobs 4
```


### Verification

All ways of multiplication produce their results in the "results" subfolder following this format:

```bash
originalFilename.results.{i}.{MODE}.dat
```

where:

- originalFilename, is the file which had the matrices pairs (e.g matrices_large.dat)
- i is the pair number (starting from 0)
- MODE: is the mode used ("REF","COARSE","FINE" or "PYTHON")

You can verify each mode against each other with the "verify" script as follows:

```bash
./verify matrices_large.dat REF PYTHON
```

