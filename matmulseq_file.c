//Matrix multiplication

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "aux_matrix_operations.h"
#include <time.h>

typedef struct {
    int jobId;
    char fname[256];
    int pair;
    // Other job-specific data
    int verbose;

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
            break; // No more jobs to process
        }

        Job job = data->jobs[data->nextJob++];
        pthread_mutex_unlock(data->mutex);

        if (job.verbose){
            printf("Thread processing job %d\n", job.jobId);
        }

        // Process the job...
        readMatrixInfo(job.fname, &nmats, &matrixSize);

        if (job.verbose){
            printf("Multiplying two matrices...\n"); //Remove this line for performance tests
        }

        //Dynamically create matrices of the size needed
        a = allocateMatrix(matrixSize);
        b = allocateMatrix(matrixSize);
        c = allocateMatrix(matrixSize);

        matrixSize=readSpecificMatrixPair(job.fname, job.pair, a, b);
        mm(a, b, c, matrixSize);
        snprintf(newFilename, sizeof(newFilename), "results/%s.result.%d.%s",job.fname, job.pair, "COARSE.dat");
        writeMatrixToFile(c,matrixSize,newFilename);
    }

    free(*a);
    free(a);
    free(*b);
    free(b);
    free(*c);
    free(c);

    return NULL;
}

int main(int argc, char *argv[]) {

    // Default values
    int defaultNThreads = 4; // Default number of threads
    char *defaultDatafile = "matrices_dev.dat"; // Default data file name
    char *defaultMode = "REF"; // Default mode
    int defaultVerbose = 0; // Default verbose mode (disabled)

    // Variables to store actual values, initialized to defaults
    int nThreads = defaultNThreads;
    char *datafile = defaultDatafile;
    char *mode = defaultMode;
    int verbose = defaultVerbose;

    // Override defaults with command line arguments if provided
    if (argc > 1) {
        nThreads = atoi(argv[1]);
    }
    if (argc > 2) {
        datafile = argv[2];
    }
    if (argc > 3) {
        mode = argv[3];
    }
    if (argc > 4) {
        verbose = atoi(argv[4]);
    }

    printf("Number of threads: %d\n", nThreads);
    printf("Datafile: %s\n", datafile);
    printf("Mode: %s\n", mode);
    printf("Verbose Mode: %d\n", verbose);


    if (verbose){
        printf("Start\n");
    }

    char *fname = datafile; //Change to matrices_large.dat for performance evaluation
    char newFilename[256]; // Adjust size as needed
    int matrixSize;
    int nmats;
    readMatrixInfo(fname, &nmats, &matrixSize);


    struct timespec start, end;
    double elapsed;

    // Start timing
    clock_gettime(CLOCK_MONOTONIC, &start);

    if (strcmp(mode,"REF")==0){
        double **a, **b, **c;
        //Dynamically create matrices of the size needed
        a = allocateMatrix(matrixSize);
        b = allocateMatrix(matrixSize);
        c = allocateMatrix(matrixSize);

        if (verbose){
            printf("Loading %d pairs of square matrices of size %d from %s...\n", nmats, matrixSize, fname);
        }

        for(int k=0;k<nmats;k++){
            matrixSize=readSpecificMatrixPair(fname, k, a, b);

            if (verbose){
                printf("Multiplying two matrices...\n"); //Remove this line for performance tests
            }
            mm(a, b, c, matrixSize);
            if (verbose){
                printResult(c,matrixSize); //Remove this line for performance tests
            }
            snprintf(newFilename, sizeof(newFilename), "results/%s.result.%d.%s",fname, k, "REF.dat");
            writeMatrixToFile(c,matrixSize,newFilename);
        }
        // Free memory
        free(*a);
        free(a);
        free(*b);
        free(b);
        free(*c);
        free(c);
    }

    if (strcmp(mode,"COARSE")==0){
        pthread_t threads[nThreads];
        pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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
            jobs[i].verbose=verbose;
        }

        // Prepare shared data
        ThreadData data;
        data.jobs = jobs;
        data.totalJobs = mJobs;
        data.nextJob = 0;
        data.mutex = &mutex;

        // Debug worker
        //coarse_worker(&data);

        // Create threads
        for (int i = 0; i < nThreads; ++i) {
            if (pthread_create(&threads[i], NULL, coarse_worker, &data) != 0) {
                perror("Failed to create thread");
                free(jobs);
                return 1;
            }
        }

        // Join threads
        for (int i = 0; i < nThreads; ++i) {
            pthread_join(threads[i], NULL);
        }

        free(jobs);
        pthread_mutex_destroy(&mutex);
    }

    // Stop timing
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate elapsed time in seconds
    elapsed = end.tv_sec - start.tv_sec;
    elapsed += (end.tv_nsec - start.tv_nsec) / 1000000000.0;

    printf("Elapsed time: %.3f seconds\n", elapsed);

    if (verbose){
        printf("Done.\n");
    }

    return 0;
}

