#include "aux_matrix_operations.h"
#include <math.h>

int matricesAreEqual(double **matrix1, double **matrix2, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (fabs(matrix1[i][j] - matrix2[i][j]) > EPSILON) {
                return 0; // Matrices are not equal
            }
        }
    }
    return 1; // Matrices are equal
}

double **allocateMatrix(int size) {
    int i;
    double *vals, **temp;

    // allocate space for values of a matrix and initialize to zero
    vals = (double *) calloc(size * size, sizeof(double));
    if (vals == NULL) {
        fprintf(stderr, "Memory allocation failed for matrix values\n");
        exit(1);
    }

    // allocate vector of pointers to create the 2D array
    temp = (double **) malloc(size * sizeof(double*));
    if (temp == NULL) {
        fprintf(stderr, "Memory allocation failed for matrix pointers\n");
        free(vals); // Free previously allocated memory
        exit(1);
    }

    for (i = 0; i < size; i++) {
        temp[i] = &vals[i * size];
    }

    return temp;
}



void mm(double **a, double **b, double **c, int matrixSize) {
    int i, j, k;
    double sum;

    // matrix multiplication
    for (i = 0; i < matrixSize; i++) {
        for (j = 0; j < matrixSize; j++) {
            sum = 0.0;
            // dot product
            for (k = 0; k < matrixSize; k++) {
                sum += a[i][k] * b[k][j];
            }
            c[i][j] = sum;
        }
    }
}

void mmSingle(double **a, double **b, double **c, int matrixSize, int i, int j) {
    int k;
    double sumA;
    double sumB;
    int diff=j!=i;
    // Single dot product
    sumA = 0.0;
    sumB = 0.0;

    if (diff){// unrolled branches to reduce conditional overhead to only 1 check
        for (k = 0; k < matrixSize; k++) {
            sumA += a[i][k] * b[k][j];
            sumB += a[j][k] * b[k][i];
        }
        c[i][j] = sumA; // there shouldnt be race conditions as each thread touches a different position...
        c[j][i] = sumB;
    }
    else{
        // dot product
        for (k = 0; k < matrixSize; k++) {
            sumA += a[i][k] * b[k][j];
        }
        c[i][j] = sumA; // there shouldnt be race conditions as each thread touches a different position...
    }
}

void printResult(double **matrix, int size) {
    int i, j;
    for(i = 0; i < size; i++) {
        for(j = 0; j < size; j++) {
            printf("%lf ", matrix[i][j]);
        }
        printf("\n");
    }
}

void writeMatrixToFile(double **matrix, int size, const char *filename) {
    // First line is size
    // Others are the matrix
    // This is a different format than the one for matrices multiplications
    // This contains only a single matrix

    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    // Write the size of the matrix at the beginning of the file
    fprintf(file, "%d\n", size);

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            fprintf(file, "%lf ", matrix[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

int readMatrixFromFile(const char *filename, double ***matrixPtr) {
    // First line is size
    // Others are the matrix
    // This is a different format than the one for matrices multiplications
    // This contains only a single matrix
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    int size;
    if (fscanf(file, "%d", &size) != 1) {
        fprintf(stderr, "Error reading matrix size from file\n");
        fclose(file);
        exit(1);
    }

    double **matrix = allocateMatrix(size);

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (fscanf(file, "%lf", &matrix[i][j]) != 1) {
                fprintf(stderr, "Error reading matrix from file\n");
                fclose(file);
                exit(1);
            }
        }
    }

    fclose(file);
    *matrixPtr = matrix;
    return size;
}

void readMatrixPair(FILE* fh, double** matrix, int matrixSize) {
    for (int i = 0; i < matrixSize; i++) {
        for (int j = 0; j < matrixSize; j++) {
            fscanf(fh, "%lf", &matrix[i][j]);
        }
    }
}

int readSpecificMatrixPair(const char* filename, int pairIndex, double** matrix1, double** matrix2) {
    FILE* fh = fopen(filename, "r");
    if (fh == NULL) {
        fprintf(stderr, "Error opening file.\n");
        exit(1);
    }

    // Skipping the first line (total pairs and size info)
    int totalPairs, matrixSize;

    // Read and save the total pairs and matrix size from the first line
    if (fscanf(fh, "%d %d\n", &totalPairs, &matrixSize) != 2) {
        fprintf(stderr, "Error reading the first line of the file\n");
        fclose(fh);
        exit(1);
    }
    // Calculate the number of elements to skip
    int elementsToSkip = pairIndex * 2 * matrixSize * matrixSize;

    // Define a temporary variable to hold the ignored value
    double dummy;

    // Skipping the unwanted pairs
    for (int i = 0; i < elementsToSkip; i++) {
        if (fscanf(fh, "%lf", &dummy) != 1) {
            // Handle or report the error
            fprintf(stderr, "Error reading file during skip\n");
            fclose(fh);
            exit(1);
        }
    }

    // Read the desired pair of matrices
    readMatrixPair(fh, matrix1, matrixSize);
    readMatrixPair(fh, matrix2, matrixSize);

    fclose(fh);
    return matrixSize;
}

void readMatrixInfo(const char* filename, int* nmats, int* matrixSize) {
    FILE* fh = fopen(filename, "r");
    if (fh == NULL) {
        fprintf(stderr, "Error opening file %s.\n", filename);
        exit(1);
    }

    if (fscanf(fh, "%d %d\n", nmats, matrixSize) != 2) {
        fprintf(stderr, "File format error.\n");
        fclose(fh);
        exit(1);
    }

    fclose(fh);
}
