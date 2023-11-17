from ctypes import CDLL, c_int,cast,addressof,sizeof
from ctypes import POINTER, c_double, c_int

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

# Function to create a C double array from a Python list
# def create_c_matrix(py_matrix, size):
#     c_matrix = (POINTER(c_double) * size)()
#     for i in range(size):
#         row = (c_double * size)(*py_matrix[i])
#         c_matrix[i] = cast(row, POINTER(c_double))
#     return c_matrix

def create_c_matrix(py_matrix, size):
    # Allocate a contiguous block for matrix data
    data = (c_double * (size * size))(*[elem for row in py_matrix for elem in row])
    
    # Allocate and set up row pointers
    c_matrix = (POINTER(c_double) * size)()
    for i in range(size):
        row_pointer = cast(addressof(data) + i * size * sizeof(c_double), POINTER(c_double))
        c_matrix[i] = row_pointer

    return c_matrix


# Load the shared library
mylib = CDLL('/home/user/code/parallelMatrixMultiplication/libaux_matrix_operations.so')
mylib.mm.argtypes = [POINTER(POINTER(c_double)), POINTER(POINTER(c_double)), POINTER(POINTER(c_double)), c_int]
mylib.mm.restype = None

# Main execution
filename = "/home/user/code/parallelMatrixMultiplication/matrices_dev.dat"
nmats, matrix_size = read_matrix_info(filename)

for pair in range(nmats):
    matrix_a, matrix_b = read_matrix_pair(filename, pair, matrix_size)
    matrix_c = [[0.0 for _ in range(matrix_size)] for _ in range(matrix_size)]

    # Convert Python matrices to C matrices
    c_matrix_a = create_c_matrix(matrix_a, matrix_size)
    c_matrix_b = create_c_matrix(matrix_b, matrix_size)
    c_matrix_c = create_c_matrix(matrix_c, matrix_size)

    # Debugging: Print a few elements of the C matrices
    print("C Matrix A[0][0]:", c_matrix_a[0][0])
    print("C Matrix B[0][0]:", c_matrix_b[0][0])
    print("C Matrix C[0][0]:", c_matrix_c[0][0])


    print("Matrix A before mm:", matrix_a)
    print("Matrix B before mm:", matrix_b)
    mylib.mm(c_matrix_a, c_matrix_b, c_matrix_c, matrix_size)
    result_matrix_c = [[c_matrix_c[i][j] for j in range(matrix_size)] for i in range(matrix_size)]
    print("Matrix C after mm:", result_matrix_c)

    # # Call the C function
    # mylib.mm(c_matrix_a, c_matrix_b, c_matrix_c, matrix_size)
    # print(matrix_a)
    # print(matrix_b)
    # print(matrix_c)

    # # matrix_c now contains the result of the matrix multiplication
    # # Process matrix_c as needed, e.g., print or save to file
