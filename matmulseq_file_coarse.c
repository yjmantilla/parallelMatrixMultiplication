//Matrix multiplication

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define NUM_THREADS 4
#define EPSILON 1e-6  // Tolerance for floating point comparison

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

    // allocate space for values of a matrix
    vals = (double *) malloc(size * size * sizeof(double));
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

    for (i = 0; i < size; i++)
        temp[i] = &vals[i * size];

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

void readSpecificMatrixPair(const char* filename, int pairIndex, int matrixSize, double** matrix1, double** matrix2) {
    FILE* fh = fopen(filename, "r");
    if (fh == NULL) {
        fprintf(stderr, "Error opening file.\n");
        exit(1);
    }

    // Skipping the first line (total pairs and size info)
    fscanf(fh, "%*d %*d\n");

    // Calculate the number of elements to skip
    int elementsToSkip = pairIndex * 2 * matrixSize * matrixSize;

    // Skipping the unwanted pairs
    for (int i = 0; i < elementsToSkip; i++) {
        fscanf(fh, "%*lf");
    }

    // Read the desired pair of matrices
    readMatrixPair(fh, matrix1, matrixSize);
    readMatrixPair(fh, matrix2, matrixSize);

    fclose(fh);
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

void *PrintHello(void *threadid)
{
   long tid;
   tid = (long)threadid;
   printf("Hello World! It's me, thread #%ld!\n", tid);
   pthread_exit(NULL);
}

int main(void) {
    pthread_t threads[NUM_THREADS];

    double **a, **b, **c, **d;
    int matrixSize;

    int i, j, k;
    int nmats;
    char *fname = "matrices_dev.dat"; //Change to matrices_large.dat for performance evaluation
    char newFilename[256]; // Adjust size as needed

    printf("Start\n");

    readMatrixInfo(fname, &nmats, &matrixSize);

    //Dynamically create matrices of the size needed
    a = allocateMatrix(matrixSize);
    b = allocateMatrix(matrixSize);
    c = allocateMatrix(matrixSize);
    d = allocateMatrix(matrixSize); // our result



    int rc;
    long t;
    for(t = 0; t < NUM_THREADS; t++) {
        printf("In main: creating thread %ld\n", t);
        rc = pthread_create(&threads[t], NULL, PrintHello, (void *)t);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    printf("Loading %d pairs of square matrices of size %d from %s...\n", nmats, matrixSize, fname);
    double **matrixREF;
    double **matrixUS;
    int equal=0;
    int matsize;

    for(k=0;k<nmats;k++){
        readSpecificMatrixPair(fname, k, matrixSize, a, b);
        printf("Multiplying two matrices...\n"); //Remove this line for performance tests
        mm(a, b, c, matrixSize);
        printResult(c,matrixSize); //Remove this line for performance tests
        snprintf(newFilename, sizeof(newFilename), "results/%s.result.%d.%s",fname, k, "REF.dat");
        writeMatrixToFile(c,matrixSize,newFilename);
        snprintf(newFilename, sizeof(newFilename), "results/%s.result.%d.%s",fname, k, "REF2.dat");
        writeMatrixToFile(c,matrixSize,newFilename);
    }


    // Verification
    for(k=0;k<nmats;k++){
        snprintf(newFilename, sizeof(newFilename), "results/%s.result.%d.%s",fname, k, "REF.dat");
        matsize=readMatrixFromFile(newFilename,&matrixREF);
        snprintf(newFilename, sizeof(newFilename), "results/%s.result.%d.%s",fname, k, "REF2.dat");
        readMatrixFromFile(newFilename,&matrixUS);
        equal=matricesAreEqual(matrixREF,matrixUS,matrixSize);
        if (equal) {
           printf("The matrices are equal for product %ld from %s.\n",k,fname);
        } else {
            printf("The matrices are not equal for product %ld from %s.\n",k,fname);
        }
    }

    // Free memory
    free(*a);
    free(a);
    free(*b);
    free(b);
    free(*c);
    free(c);
    printf("Done.\n");

    // Verification


    return 0;
}

