// ----------------------------------------------------------------------------
// File:        Parallel.cpp
// Author:      Codey Funston
// Version:     1.0.0 
//
// Description: 
//             
//              Parallel implementation of Sequential.cpp using pthreads.
//
//              *Note: Funnily, in all my previous files that used pthreads
//              I forgot to include the header pthread.h. However, it turns
//              out that somewhere in iostream or recursively within other
//              headers in iostream pthread.h is included, so my files 
//              compiled and accessed multiple threads fine. I have included it
//              in this file for good practice!
//
// ----------------------------------------------------------------------------

#include <chrono>
#include <iomanip>
#include <iostream>
#include <time.h> 
#include <chrono>
#include <fstream>
#include <string>
#include <unistd.h>
#include <pthread.h> // :)

using namespace std::chrono;
using namespace std;


const int N = 100; // N is number of rows and columns.
const size_t MAX_ENTRY_VALUE = 100;
const long NUM_CORES = sysconf(_SC_NPROCESSORS_ONLN);
const size_t PARTITION_SIZE = N / 20;

using Matrix = int**;

struct ThreadDataFill {
    Matrix m;
    size_t index
}

// Sets each value in a matrix to a random integer between
// [0, MAX_ENTRY_VALUE].
void randomMatrix(Matrix M) {
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            // *Note: In previous submissions I made an error with the modulo
            // operator as x % n is between [0, n) ie non-inclusive. This is
            // why I have added a +1.
            M[i][j] = rand() % MAX_ENTRY_VALUE + 1;
        }
    } 
}

// Computes the dot product of two square matrices, resulting in matrix C.
void dotProduct(Matrix A, Matrix B, Matrix C) {
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
        << "Sequential Runtime (\xC2\xB5s):          " // UTF-8 for "micro".
        << duration.count() << " microseconds" 
        << endl << endl;

    matrixFile(A, "matrixA.mtrx");
    matrixFile(B, "matrixB.mtrx");
    matrixFile(C, "matrixC.mtrx");

    return EXIT_SUCCESS;
}
