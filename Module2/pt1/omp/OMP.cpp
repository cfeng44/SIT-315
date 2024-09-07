// ----------------------------------------------------------------------------
// File:        OMP.cpp
// Author:      Codey Funston
// Version:     1.1.0 
//
// Description: 
//             
//              Parallel implementation of Sequential.cpp using OMP.
//
// ----------------------------------------------------------------------------

#include <chrono>
#include <iomanip>
#include <iostream>
#include <time.h> 
#include <chrono>
#include <fstream>
#include <string>
#include <omp.h>

using namespace std::chrono;
using namespace std;


const int N = 100; // N is number of rows and columns.
const size_t MAX_ENTRY_VALUE = 100;
const long NUM_CORES = omp_get_num_procs();
const size_t PARTITION_SIZE = N*N / 20;

using Matrix = int**;

// Sets each value in a matrix to a random integer between
// [0, MAX_ENTRY_VALUE].
void randomMatrix(Matrix M) {
    // The omp pragma attempts to replicate the parallel structure of the pthreads version.
    #pragma omp parallel for collapse(2) num_threads(NUM_CORES) schedule(dynamic, PARTITION_SIZE)
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            M[i][j] = rand() % MAX_ENTRY_VALUE + 1;
        }
    } 
}

// Computes the dot product of two square matrices, resulting in matrix C.
void dotProduct(Matrix A, Matrix B, Matrix C) {
    // This will include the loops for rows and columns, but not the one for
    // the inner loop since this is where we don't want parallelism.
    #pragma omp parallel for collapse(2) num_threads(NUM_CORES) schedule(dynamic, PARTITION_SIZE)
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            C[i][j] = 0; // Set to 0 initially so we can increment it below.
            for (size_t k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

// Prints a matrix to an output stream, formatted in the way commonly used in
// mathematics: 
// [a b c]
// [d e f]
// [g h i]
void printMatrix(Matrix m, ostream& stream) {
    for (size_t i = 0; i < N; i++) {
        stream << "[";
        for (size_t j = 0; j < N; j++) {
            if (j != N -1) {
                // Setting width to the number of digits in MAX_ENTRY_VALUE 
                // ensures that all the values in the output are evenly spaced.
                int digit = to_string(MAX_ENTRY_VALUE).length();
                stream << setw(digit) <<  m[i][j] << " ";
            }
            else {
                // Gets rid of extra space for last element in each row.
                int digit = to_string(MAX_ENTRY_VALUE).length();
                stream << setw(digit) <<  m[i][j];
            }
        }
        stream << "]" << endl;
    }
}

// Prints a matrix to a file.
void matrixFile(Matrix m, string file_name) {
    ofstream results_file;
    results_file.open(file_name);
    printMatrix(m, results_file);
    results_file.close();
}

int main() {
    // Sets the random seed to a different value each time the program executes.
    // Time(0) returns the current time since Epoch time (Jan 1 1970).
    srand(time(0));
    cout << NUM_CORES << "<-- HERE\n";
    int **A, **B, **C;
    
    A = new int*[N];
    B = new int*[N];
    C = new int*[N];

    for (size_t i = 0; i < N; i++) {
        A[i] = new int [N];
        B[i] = new int [N];
        C[i] = new int [N];
    }

    // Saves the start time.
    auto start = high_resolution_clock::now();
    
    randomMatrix(A);
    randomMatrix(B);
    dotProduct(A, B, C);

    // Saves the finish time.
    auto finish = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(finish - start);

    cout << endl
        << "OMP Runtime (\xC2\xB5s):                 " // UTF-8 for "micro".
        << duration.count() << " microseconds" 
        << endl << endl;

    matrixFile(A, "OMP_matrixA.mtrx");
    matrixFile(B, "OMP_matrixB.mtrx");
    matrixFile(C, "OMP_matrixC.mtrx");

    return EXIT_SUCCESS;
}
