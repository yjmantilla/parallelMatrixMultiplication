#ifndef MATRIX_OPERATIONS_H
#define MATRIX_OPERATIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define EPSILON 1e-6  // Tolerance for floating point comparison


// Place all function declarations here
double **allocateMatrix(int size);
void mm(double **a, double **b, double **c, int matrixSize);
void mmFine(double **a, double **b, double **c, int matrixSize, int* is, int*  js,int start,int end);
void mmFineHungry(double ***a, double ***b, double ***c, int matrixSize, int* is, int* js,int start,int end, int nmats);
void mmSingleFine(double ***a, double ***b, double ***c, int matrixSize, int* is, int*  js,int start,int end, int pair);
void printResult(double **matrix, int size);
void writeMatrixToFile(double **matrix, int size, const char *filename);
int readMatrixFromFile(const char *filename, double ***matrixPtr);
void readMatrixPair(FILE* fh, double** matrix, int matrixSize);
int readSpecificMatrixPair(const char* filename, int pairIndex, double** matrix1, double** matrix2);
void readMatrixInfo(const char* filename, int* nmats, int* matrixSize);
int matricesAreEqual(double **matrix1, double **matrix2, int size);

#endif // MATRIX_OPERATIONS_H
