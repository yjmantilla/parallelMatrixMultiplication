from ctypes import CDLL, c_int,cast,addressof,sizeof
from ctypes import POINTER, c_double, c_int
import numpy as np
import os
from joblib import Parallel, delayed
import argparse
import time

# Create the parser
parser = argparse.ArgumentParser(description="Parallel Matrix Multiplication")

# Add arguments
parser.add_argument('filename', type=str, default="/home/user/code/parallelMatrixMultiplication/matrices_dev.dat",help='Path to the matrix file')
parser.add_argument('--n_jobs', type=int, default=4, help='Number of jobs to run in parallel (default: 4)')
parser.add_argument('--verbose', action='store_true', help='Enable verbose output')

# Parse arguments
args = parser.parse_args()

# Use the parsed arguments
filename = args.filename
n_jobs = args.n_jobs
verbose = args.verbose

def read_matrix_info(filename):
    with open(filename, 'r') as file:
        first_line = file.readline()
        nmats, matrix_size = map(int, first_line.split())
    return nmats, matrix_size

def read_matrix_pair(filename, pair_index, matrix_size):
    matrix1 = [[0.0 for _ in range(matrix_size)] for _ in range(matrix_size)]
    matrix2 = [[0.0 for _ in range(matrix_size)] for _ in range(matrix_size)]

    with open(filename, 'r') as file:
        # Skip the first line (total pairs and matrix size)
        next(file)

        # Calculate the number of lines to skip to reach the desired pair
        lines_to_skip = pair_index * 2 * matrix_size
        for _ in range(lines_to_skip):
            next(file)

        # Read the first matrix of the pair
        for i in range(matrix_size):
            matrix1[i] = list(map(float, file.readline().split()))

        # Read the second matrix of the pair
        for i in range(matrix_size):
            matrix2[i] = list(map(float, file.readline().split()))

    return matrix1, matrix2

def create_c_matrix_old(py_matrix, size):
    """The issue with the first element of result_matrix_a being incorrect (6.94355611453995e-310 instead of 2.0) suggests a problem with how the memory is addressed or cast in the create_c_matrix function. This might be caused by an incorrect offset calculation for the start of each row.
    Let's try a different approach to allocate and set up the c_matrix:

    Updated create_c_matrix Function Using numpy
    We can use numpy to create the contiguous block of memory and then set up the c_matrix using numpy's ability to interface with C arrays. numpy arrays ensure proper memory management and alignment, which can help avoid issues with raw pointer arithmetic.
    """
    # Allocate a contiguous block for matrix data
    data = (c_double * (size * size))(*[elem for row in py_matrix for elem in row])
    
    # Allocate and set up row pointers
    c_matrix = (POINTER(c_double) * size)()
    for i in range(size):
        row_pointer = cast(addressof(data) + i * size * sizeof(c_double), POINTER(c_double))
        c_matrix[i] = row_pointer

    return c_matrix,None

def create_c_matrix(py_matrix, size):
    # Create a numpy array with the matrix data
    np_matrix = np.array(py_matrix, dtype=c_double, order='C')

    # Create an array of pointers to the rows of the numpy array
    c_matrix = (POINTER(c_double) * size)()
    for i in range(size):
        c_matrix[i] = np_matrix[i].ctypes.data_as(POINTER(c_double))

    return c_matrix, np_matrix  # Return both to keep numpy array in scope

def write_matrix_to_file(matrix, size, filename):
    # Open the file for writing
    with open(filename, 'w') as file:
        # Write the size of the matrix at the beginning of the file
        file.write(f"{size}\n")

        # Write the matrix
        for i in range(size):
            for j in range(size):
                file.write(f"{matrix[i][j]} ")
            file.write("\n")


def process_matrix_pair(pair, filename, verbose):
    # Load the shared library
    mylib = CDLL('/home/user/code/parallelMatrixMultiplication/libaux_matrix_operations.so')
    mylib.mm.argtypes = [POINTER(POINTER(c_double)), POINTER(POINTER(c_double)), POINTER(POINTER(c_double)), c_int]
    mylib.mm.restype = None

    nmats, matrix_size = read_matrix_info(filename)
    verbose=False

    matrix_a, matrix_b = read_matrix_pair(filename, pair, matrix_size)
    matrix_c = [[0.0 for _ in range(matrix_size)] for _ in range(matrix_size)]

    c_matrix_a, np_matrix_a = create_c_matrix(matrix_a, matrix_size)
    c_matrix_b, np_matrix_b = create_c_matrix(matrix_b, matrix_size)
    c_matrix_c, np_matrix_c = create_c_matrix(matrix_c, matrix_size)

    if verbose:
        print("Matrix A:", matrix_a)
        print("Matrix B:", matrix_b)

    mylib.mm(c_matrix_a, c_matrix_b, c_matrix_c, matrix_size)
    result_matrix_c = [[c_matrix_c[i][j] for j in range(matrix_size)] for i in range(matrix_size)]

    fname = os.path.basename(filename)
    dname = os.path.dirname(filename)
    new_filename = f"{dname}/results/{fname}.result.{pair}.PYTHON.dat"
    write_matrix_to_file(result_matrix_c, matrix_size, new_filename)


nmats, matrix_size = read_matrix_info(filename)
verbose = False

# Parallel processing

# Start timing
start_time = time.time()
Parallel(n_jobs=n_jobs)(delayed(process_matrix_pair)(pair, filename, verbose) for pair in range(nmats))
# End timing
end_time = time.time()

# Calculate elapsed time
elapsed_time = end_time - start_time

print(f"Elapsed time: {elapsed_time} seconds")
