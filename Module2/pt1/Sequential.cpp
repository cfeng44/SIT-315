// ----------------------------------------------------------------------------
// File:        Sequential.cpp
// Author:      Codey Funston
// Version:     1.0.0 
//
// Description: 
//             
//              Multiplying square matrices (NxN) that are filled with random
//              integers. Using modern C++ (>C++11) features now after getting
//              pointers and memory experience with C-style in previous code
//              files.
//
//              All operations are implemented with sequential programming.
//
// ----------------------------------------------------------------------------

#include <chrono>
#include <iomanip>
#include <iostream>
#include <time.h> 
#include <chrono>

using namespace std::chrono;
using namespace std;


const size_t N = 100; // N is number of rows and columns.

using Matrix = int**;

void randomMatrix(Matrix M) {
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            // Initialises vector entry to random number in [0, 100].
            M[i][j] = rand() % 100;
        }
    } 
}

void dotProduct(Matrix A, Matrix B, Matrix C) {
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            for (size_t k = 0; k < N; k++) {
                C[i][j] += A[i][j] * B[k][j];
            }
        }
    }
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

    return EXIT_SUCCESS;
}
