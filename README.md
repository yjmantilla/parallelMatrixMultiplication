# parallelMatrixMultiplication

2nd Lab of advanced computer archs udea.

Se puede hacer dinamico o estatico (en el sentido antes de que empiece el trabajo paralelo, pero parametrizable para matrices NxN y T Threads).

## Primero hacer el script de verificacion de la multiplicacion de matrices (resultados)

No tirarse la semantica de los calculos que tenmos que hacer por race conditions

Puede ser una funcion del codigo.


## Instructions

### Install required python libraries

```bash
pip install numpy
pip install joblib
pip install pandas
```

### Compile using make

```bash
make
```

### Run timing experiments

python3 timing_experiments.py

### Running individual codes

There are four ways to run the multiplications:

1. C-based "REF" (the one provided by the course)
1. C-based "COARSE" (runs a single multiplication of pairs per thread until there are none left)
1. C-based "FINE" (runs a single row*column product of the 2 matrices per thread)
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

