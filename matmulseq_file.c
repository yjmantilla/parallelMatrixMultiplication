//Matrix multiplication

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "aux_matrix_operations.h"
#include <time.h>
#include <stdbool.h>

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

    pthread_mutex_lock(data->mutex);
    Job job = data->jobs[data->nextJob];
    // asumme same matrixsize
    readMatrixInfo(job.fname, &nmats, &matrixSize); // it seems to be more safe to open the file in order
    pthread_mutex_unlock(data->mutex);

    //Dynamically create matrices of the size needed
    a = allocateMatrix(matrixSize);
    b = allocateMatrix(matrixSize);
    c = allocateMatrix(matrixSize);

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


typedef struct {
    int jobId;
    int pair;
    // Other job-specific data
    int verbose;
    int i;
    int j;
    double **matrixRefA; // Reference to a matrix (double pointer)
    double **matrixRefB; // Reference to a matrix (double pointer)
    double **matrixRefC; // Reference to a matrix (double pointer)
    int matrixSize;
} FineJob;

typedef struct {
    int totalJobs;
    int nextJob;
    int jobsCompleted;
    bool terminate;
    FineJob *jobs;
    pthread_cond_t *jobAvailableCond;
    pthread_mutex_t *mutex;
} FineThreadData;


void *fine_worker(void *arg) {
    FineThreadData *data = (FineThreadData *)arg;

    while (1) {
        pthread_mutex_lock(data->mutex);

        // Wait while there are no jobs and the thread should not terminate
        while (data->nextJob >= data->totalJobs && !data->terminate) {
            // jobCompleted should be greaater == to totalJobs here
            pthread_cond_wait(data->jobAvailableCond, data->mutex);
        }

        // Break the loop if the thread should terminate
        if (data->terminate) {
            pthread_mutex_unlock(data->mutex);
            break;
        }

        // Get the job and increment the nextJob index
        FineJob job = data->jobs[data->nextJob++];
        pthread_mutex_unlock(data->mutex);

        // Process the job
        mmSingle(job.matrixRefA, job.matrixRefB, job.matrixRefC, job.matrixSize, job.i, job.j);

        pthread_mutex_lock(data->mutex);
        data->jobsCompleted++;

        // Signal that all jobs are done if this is the last job
        if (data->jobsCompleted == data->totalJobs) {
            ;
        }
        pthread_mutex_unlock(data->mutex);
    }
    return NULL;
}


typedef struct {
    int jobId;
    int pair;
    // Other job-specific data
    int verbose;
    int i;
    int j;
    double **matrixRefA; // Reference to a matrix (double pointer)
    double **matrixRefB; // Reference to a matrix (double pointer)
    double **matrixRefC; // Reference to a matrix (double pointer)
    int matrixSize;
} FineHungryJob;

typedef struct {
    int totalJobs;
    int nextJob;
    int jobsCompleted;
    FineJob *jobs;
    pthread_mutex_t *mutex;
} FineHungryThreadData;


void *fineHungry_worker(void *arg) {
    FineHungryThreadData *data = (FineHungryThreadData *)arg;
    while (1) {
        pthread_mutex_lock(data->mutex);
        if (data->nextJob >= data->totalJobs) {
            pthread_mutex_unlock(data->mutex);
            break;
        }

        FineHungryJob *job = &data->jobs[data->nextJob];
        data->nextJob++;
        pthread_mutex_unlock(data->mutex);

        if (job->verbose){
            printf("Thread processing job %d\n", job->jobId);
        }

        // Process the job...
        mmSingle(job->matrixRefA, job->matrixRefB, job->matrixRefC, job->matrixSize, job->i, job->j);
    }
    return NULL;
}

int main(int argc, char *argv[]) {

    // Default values
    int defaultNThreads = 1; // Default number of threads
    char defaultDatafile[256] = "matrices_dev.dat"; // Default data file name
    char defaultMode[20] = "FINE"; // Default mode
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

        if (nThreads > mJobs){
            nThreads = mJobs;
        }

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


    if (strcmp(mode,"FINE")==0){
        pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

        // Allocate matrices outside of the worker threads
        double **a = allocateMatrix(matrixSize);
        double **b = allocateMatrix(matrixSize);
        double **c = allocateMatrix(matrixSize);
        
        matrixSize=readSpecificMatrixPair(fname, 0, a, b);
        int mJobs = matrixSize*(matrixSize+1)/2;

        FineJob *jobs = malloc(mJobs * sizeof(FineJob));

        if (!jobs) {
            perror("Failed to allocate memory for jobs");
            return 1;
        }

        // Initialize jobs
        int jobNum =0;
        for (int i = 0; i < matrixSize; i++) {
            for (int j = i; j < matrixSize; j++){
                jobs[jobNum].verbose=verbose;
                jobs[jobNum].i=i;
                jobs[jobNum].j=j;
                jobs[jobNum].matrixRefA=a;
                jobs[jobNum].matrixRefB=b;
                jobs[jobNum].matrixRefC=c;
                jobs[jobNum].matrixSize=matrixSize;
                //printf("%d %d %d \n",jobNum,i,j);
                jobNum++;
            }
        }

        if (mJobs!=jobNum){
            return 1;
        }


        // Prepare shared data
        FineThreadData data;
        pthread_cond_t jobAvailableCond = PTHREAD_COND_INITIALIZER;
        data.mutex = &mutex;
        data.jobAvailableCond = &jobAvailableCond;
        data.terminate = false;
        data.jobs = jobs;
        data.nextJob = 0;
        data.jobsCompleted = 0; // Reset jobsCompleted for the new batch
        data.totalJobs = mJobs;

        // Debug worker
        //fine_worker(&data);

        if (nThreads > mJobs){
            nThreads = mJobs;
        }

        pthread_t threads[nThreads];

        for (int i = 0; i < nThreads; ++i) {
            if (pthread_create(&threads[i], NULL, fine_worker, &data) != 0) {
                perror("Failed to create thread");
                free(jobs);
                return 1;
            }
        }


        for (int k = 0; k < nmats; ++k) {
            // Read and process the matrix pair
            readSpecificMatrixPair(fname, k, a, b);

            pthread_mutex_lock(&mutex);
            data.nextJob = 0;
            data.jobsCompleted = 0;
            data.totalJobs = mJobs; // Ensure this is set correctly for each batch
            pthread_mutex_unlock(&mutex);

            pthread_cond_broadcast(&jobAvailableCond);

            // Wait for jobs to complete

            while (data.jobsCompleted < data.totalJobs) {
                ;
            }

            // Output results, etc.
            snprintf(newFilename, sizeof(newFilename), "results/%s.result.%d.%s", fname, k, "FINE.dat");
            writeMatrixToFile(c, matrixSize, newFilename);
        }

        // Signal to threads that all jobs are complete
        pthread_mutex_lock(&mutex);
        data.terminate = true;
        pthread_mutex_unlock(&mutex);
        pthread_cond_broadcast(&jobAvailableCond); // Signal threads to exit

        // Join threads
        for (int i = 0; i < nThreads; ++i) {
            pthread_join(threads[i], NULL);
        }

        // Clean up and free resources
        free(*a);
        free(a);
        free(*b);
        free(b);
        free(*c);
        free(c);
        free(jobs);

        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&jobAvailableCond);
        //END
    }
    if (strcmp(mode,"FINEHUNGRY")==0){

        pthread_mutex_t mutex;
        pthread_mutex_init(&mutex, NULL);

        int mJobs = nmats*matrixSize*(matrixSize+1)/2;

        FineHungryJob *jobs = malloc(mJobs * sizeof(FineHungryJob));
        if (!jobs) {
            perror("Failed to allocate memory for jobs");
            return 1;
        }

        if (nThreads > mJobs){
            nThreads = mJobs;
        }

        pthread_t threads[nThreads];

        // Allocation

        double ***matricesA = (double ***)malloc(nmats * sizeof(double **));
        double ***matricesB = (double ***)malloc(nmats * sizeof(double **));
        double ***matricesC = (double ***)malloc(nmats * sizeof(double **));

        if (matricesA == NULL) {
            perror("Memory allocation failed for matricesA array");
            exit(1);
        }

        if (matricesB == NULL) {
            perror("Memory allocation failed for matricesB array");
            exit(1);
        }

        if (matricesC == NULL) {
            perror("Memory allocation failed for matricesC array");
            exit(1);
        }

        // Allocate each matrix
        for (int i = 0; i < nmats; i++) {
            matricesA[i] = allocateMatrix(matrixSize);
            matricesB[i] = allocateMatrix(matrixSize);
            matricesC[i] = allocateMatrix(matrixSize);
        }

        // Read Matrices
        for(int k=0;k<nmats;k++){
            readSpecificMatrixPair(fname, k, matricesA[k], matricesB[k]);
        }

        // Initialize jobs
        int jobNum =0;
        for (int pair=0; pair < nmats; pair++){
            for (int i = 0; i < matrixSize; i++) {
                for (int j = i; j < matrixSize; j++){
                    jobs[i].jobId = jobNum; // Example job initialization
                    jobs[jobNum].pair=pair;
                    jobs[jobNum].verbose=verbose;
                    jobs[jobNum].i=i;
                    jobs[jobNum].j=j;
                    jobs[jobNum].matrixRefA=matricesA[pair];
                    jobs[jobNum].matrixRefB=matricesB[pair];
                    jobs[jobNum].matrixRefC=matricesC[pair];
                    jobs[jobNum].matrixSize=matrixSize;
                    jobNum++;
                }
            }
        }

        // Prepare shared data
        FineHungryThreadData data;
        data.jobs = jobs;
        data.totalJobs = mJobs;
        data.nextJob = 0;
        data.mutex = &mutex;
        // this are not used in this mode though

        // Debug worker
        //fine_worker(&data);

        // Create threads
        for (int i = 0; i < nThreads; ++i) {
            if (pthread_create(&threads[i], NULL, fineHungry_worker, &data) != 0) {
                perror("Failed to create thread");
                free(jobs);
                return 1;
            }
        }

        // Join threads
        for (int i = 0; i < nThreads; ++i) {
            pthread_join(threads[i], NULL);
        }
        //Write Results

        for(int k=0;k<nmats;k++){
            snprintf(newFilename, sizeof(newFilename), "results/%s.result.%d.%s",fname, k, "FINEHUNGRY.dat");
            writeMatrixToFile(matricesC[k],matrixSize,newFilename);
        }

        free(jobs);
        for (int i = 0; i < nmats; i++) {
            // Free the contiguous block allocated for each matrix in matricesA, matricesB, and matricesC
            free(matricesA[i][0]);  // Free the block of matrix elements
            free(matricesA[i]);     // Free the array of pointers

            free(matricesB[i][0]);  // Repeat for matricesB
            free(matricesB[i]);

            free(matricesC[i][0]);  // Repeat for matricesC
            free(matricesC[i]);
        }

        // Finally, free the top-level arrays of pointers
        free(matricesA);
        free(matricesB);
        free(matricesC);
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

