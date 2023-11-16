# Parallel Programming Assignment

## Goals

- To learn how to write simple parallel programs
- To learn the basics of the Pthreads API in C
- To analyze the impact of decomposition granularity in parallel programs
- To measure the scalability of a parallel program

## Assignment

A C program that implements sequential square matrix multiplication is provided to you. It reads several pairs of matrices from a file and computes their multiplication. Your task is to develop parallel versions of the program using the Pthreads API and the C language.

- Coarse-grain version: Parallelize the outer loop in the main() function in order to let each thread perform at least a whole matrix multiplication.

- Fine-grain version: Parallelize the inside of the mm() function in order to perform every single matrix multiplication collaboratively among multiple parallel threads.

For both versions, develop the code in such a way that the number of threads is a parameter (easy to vary) and the algorithm distributes the work as balanced as possible according to the available threads and size of the input data.

To facilitate debugging, start by creating a function that compares the result matrices of the parallelized code with the ones computed by the sequential version. They should match. Develop the code using the matrices_dev.dat file that contains two pairs of small matrices.

After making sure the parallel version is functionally correct, run multiple experiments where you vary the number of threads from 1 to 32 and measure the execution time (use a multicore machine with as many cores as possible). Use the input file matrices_large.dat when doing time measurements.

## Optional

Develop a third parallel version in which you combine Python and C. The parallelism should be implemented in Python in a similar way as the coarse-grain version and you can use any Python library to achieve parallelism. The matrix multiplication should be in C, using the same body function provided although the prototype can be changed. Investigate how to achieve such integration of Python and C into a single program.

## Deliverables

- A link to a repo containing all source codes developed.
- A 2-page report with a single plot showing execution time vs. number of threads for the two parallel versions and the sequential baseline. The report should briefly describe the work done and relevant conclusions. Finally, it should also provide relevant information about the machine where the time measurements were taken.
- Bring the report printed on paper for the oral exam about the assignment. Upload the report to the course website by the specified date. Assignments submitted at a later date will have a proportional grade penalization.