
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aux_matrix_operations.h"

int main(int argc, char *argv[]) {

    // Default values
    char *defaultDatafile = "matrices_dev.dat"; // Default data file name
    char *defaultMode1 = "REF"; // Default mode1
    char *defaultMode2 = "COARSE"; // Default mode2

    char *datafile = defaultDatafile;
    char *mode1 = defaultMode1;
    char *mode2 = defaultMode2;


    // Override defaults with command line arguments if provided
    if (argc > 1) {
        datafile = argv[1];
    }
    if (argc > 2) {
        mode1 = argv[2];
    }
    if (argc > 3) {
        mode1 = argv[3];
    }
    printf("Datafile: %s\n", datafile);
    printf("Mode1: %s\n", mode1);
    printf("Mode2: %s\n", mode2);


    char *fname = datafile; //Change to matrices_large.dat for performance evaluation
    char newFilename[256]; // Adjust size as needed
    int matrixSize;
    int nmats;
    readMatrixInfo(fname, &nmats, &matrixSize);
    double **MAT1, **MAT2;
    //Dynamically create matrices of the size needed
    MAT1 = allocateMatrix(matrixSize);
    MAT2 = allocateMatrix(matrixSize);
    int equal;
    // Verification
    for(int k=0;k<nmats;k++){
        snprintf(newFilename, sizeof(newFilename), "results/%s.result.%d.%s.dat",fname, k, mode1);
        matrixSize=readMatrixFromFile(newFilename,&MAT1);
        snprintf(newFilename, sizeof(newFilename), "results/%s.result.%d.%s.dat",fname, k, mode2);
        readMatrixFromFile(newFilename,&MAT2);
        equal=matricesAreEqual(MAT1,MAT2,matrixSize);
        if (equal) {
           printf("The matrices are equal for product %d from %s.\n",k,fname);
        } else {
            printf("The matrices are not equal for product %d from %s.\n",k,fname);
        }
    }
}
