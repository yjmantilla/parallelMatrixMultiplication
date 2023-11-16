//Matrix multiplication

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "matrix_operations.h"

#define N_THREADS 4

typedef struct {
    int jobId;
    char fname[256];
    int pair;
    // Other job-specific data
} Job;

typedef struct {
    Job *jobs;
    int totalJobs;
    int nextJob;
    pthread_mutex_t *mutex;
} ThreadData;

void *coarse_worker(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    char newFilename[256]; // Adjust size as needed
    double **a, **b, **c;
    int matrixSize;
    int nmats;

    while (1) {
        pthread_mutex_lock(data->mutex);
        if (data->nextJob >= data->totalJobs) {
            pthread_mutex_unlock(data->mutex);
            free(*a);
            free(a);
            free(*b);
            free(b);
            free(*c);
            free(c);
            break; // No more jobs to process
        }

        Job job = data->jobs[data->nextJob++];
        pthread_mutex_unlock(data->mutex);

        printf("Thread processing job %d\n", job.jobId);

        // Process the job...
        readMatrixInfo(job.fname, &nmats, &matrixSize);

        printf("Multiplying two matrices...\n"); //Remove this line for performance tests
        //Dynamically create matrices of the size needed
        a = allocateMatrix(matrixSize);
        b = allocateMatrix(matrixSize);
        c = allocateMatrix(matrixSize);

        matrixSize=readSpecificMatrixPair(job.fname, job.pair, a, b);
        mm(a, b, c, matrixSize);
        snprintf(newFilename, sizeof(newFilename), "results/%s.result.%d.%s",job.fname, job.pair, "COARSE.dat");
        writeMatrixToFile(c,matrixSize,newFilename);
    }

    return NULL;
}

int main(void) {
    pthread_t threads[N_THREADS];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    double **a, **b, **c;
    int matrixSize;

    int nmats;

    char *fname = "matrices_large.dat"; //Change to matrices_large.dat for performance evaluation
    char newFilename[256]; // Adjust size as needed

    printf("Start\n");

    readMatrixInfo(fname, &nmats, &matrixSize);
    int mJobs = nmats;

    Job *jobs = malloc(mJobs * sizeof(Job));
    if (!jobs) {
        perror("Failed to allocate memory for jobs");
        return 1;
    }


    // Initialize jobs
    for (int i = 0; i < mJobs; ++i) {
        jobs[i].jobId = i; // Example job initialization

    // Ensure the destination buffer is large enough
    if (strlen(fname) < sizeof(jobs[i].fname)) {
        strcpy(jobs[i].fname, fname);
    } else {
        fprintf(stderr, "Error: Source filename is too long to copy\n");
        // Handle the error, perhaps exit or assign a default value
        exit(1); // or handle as needed
    }

        jobs[i].pair=i;
    }


    // Prepare shared data
    ThreadData data;
    data.jobs = jobs;
    data.totalJobs = mJobs;
    data.nextJob = 0;
    data.mutex = &mutex;

    


    //Dynamically create matrices of the size needed
    a = allocateMatrix(matrixSize);
    b = allocateMatrix(matrixSize);
    c = allocateMatrix(matrixSize);

    printf("Loading %d pairs of square matrices of size %d from %s...\n", nmats, matrixSize, fname);
    double **matrixREF;
    double **matrixUS;
    int equal=0;
    int matsize;

    for(int k=0;k<nmats;k++){
        matrixSize=readSpecificMatrixPair(fname, k, a, b);
        printf("Multiplying two matrices...\n"); //Remove this line for performance tests
        mm(a, b, c, matrixSize);
        printResult(c,matrixSize); //Remove this line for performance tests
        snprintf(newFilename, sizeof(newFilename), "results/%s.result.%d.%s",fname, k, "REF.dat");
        writeMatrixToFile(c,matrixSize,newFilename);
    }

    // Debug worker
    //coarse_worker(&data);


    // Create threads
    for (int i = 0; i < N_THREADS; ++i) {
        if (pthread_create(&threads[i], NULL, coarse_worker, &data) != 0) {
            perror("Failed to create thread");
            free(jobs);
            return 1;
        }
    }

    // Join threads
    for (int i = 0; i < N_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }

    free(jobs);
    pthread_mutex_destroy(&mutex);

    // Verification
    for(int k=0;k<nmats;k++){
        snprintf(newFilename, sizeof(newFilename), "results/%s.result.%d.%s",fname, k, "REF.dat");
        matsize=readMatrixFromFile(newFilename,&matrixREF);
        snprintf(newFilename, sizeof(newFilename), "results/%s.result.%d.%s",fname, k, "COARSE.dat");
        readMatrixFromFile(newFilename,&matrixUS);
        equal=matricesAreEqual(matrixREF,matrixUS,matrixSize);
        if (equal) {
           printf("The matrices are equal for product %d from %s.\n",k,fname);
        } else {
            printf("The matrices are not equal for product %d from %s.\n",k,fname);
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

