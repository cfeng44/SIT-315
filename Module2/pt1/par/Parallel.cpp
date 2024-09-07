// ----------------------------------------------------------------------------
// File:        Parallel.cpp
// Author:      Codey Funston
// Version:     2.0.0: Huge change compared to 1.2.2. Moved to giving threads 
//              rows instead of a length that is traversed ie (1, 3) to (4, 0).
//              because so many problems were occurring and there was 
//              unnecessary complexity.
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


const size_t N = 100; // N is number of rows and columns.
const size_t MAX_ENTRY_VALUE = 100;
const long NUM_CORES = sysconf(_SC_NPROCESSORS_ONLN);
const size_t PARTITION_SIZE = N / 20;

using Matrix = int**;

struct Section {
    size_t start;
    size_t end;
};

struct ThreadDataFill {
    Matrix m;
    size_t *index;
    pthread_mutex_t *mutex;
};

struct ThreadDataDotP {
    Matrix m1;
    Matrix m2;
    Matrix m3;
    size_t *index;
    pthread_mutex_t *mutex;
};

// --- Matrix Operation Functions ---

// Set entries in each row between [s.start, s.end] with random values.

/* 
 * Sets entries in each row between [s.start, s.end] with random values.
 * @param M Matrix of **int type.
 * @param s Section in the vector. Has start (size_t) and end (size_t).
 */
void randomMatrix(Matrix M, Section s) {
    for (size_t row = s.start; row < s.end; row++) {
        for (size_t col = 0; col < N; col++) {
            M[row][col] = rand() % MAX_ENTRY_VALUE + 1;
        }
    }
}

// Computes the dot product of two square matrices, resulting in matrix C.
void dotProduct(Matrix A, Matrix B, Matrix C, Section s) {
    for (size_t row = s.start; row < s.end; row++) {
        for (size_t col = 0; col < N; col++) {
            C[row][col] = 0; // Set to 0 initially so we can increment it below.
            for (size_t k = 0; k < N; k++) {
                C[row][col] += A[row][k] * B[k][col];
            }
        }
    }
}

/*
* Prints a matrix to an output stream, formatted in the way commonly used in
* mathematics: 
* [a b c]
* [d e f]
* [g h i]
*
* @param m Matrix type (**int)
* @returns void
*/
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

// --- Threading Functions ---

void *matrixFill(void *arg) {
    ThreadDataFill *data = static_cast<ThreadDataFill *>(arg);

    Section s;
    while (true) {
        pthread_mutex_lock(data->mutex); // Position locked.
        s.start = *data->index;
        *data->index += PARTITION_SIZE;
        pthread_mutex_unlock(data->mutex);

        if (s.start >= N) {
            break;
        }

        s.end = min(s.start + PARTITION_SIZE, N);
        
        randomMatrix(data->m, s);
    }

    pthread_exit(nullptr);
}

void *matrixDotP(void *arg) {
    ThreadDataDotP *data = static_cast<ThreadDataDotP *>(arg);

    Section s;
    while (true) {
        pthread_mutex_lock(data->mutex);
        s.start = *data->index;
        *data->index += PARTITION_SIZE;
        pthread_mutex_unlock(data->mutex);

        if (s.start >= N) {
            break;
        }

        s.end = min(s.start + PARTITION_SIZE, N);

        dotProduct(data->m1, data->m2, data->m3, s);
    }

    pthread_exit(nullptr);
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
        A[i] = new int [N]();
        B[i] = new int [N]();
        C[i] = new int [N]();
    }

    // Saves the start time.
    auto start= high_resolution_clock::now();

    pthread_t tid[NUM_CORES];
    ThreadDataFill *fill_data;
    ThreadDataDotP *dot_p_data;
    fill_data = new ThreadDataFill [NUM_CORES];
    dot_p_data = new ThreadDataDotP [NUM_CORES];

    size_t m1_index = 0, m2_index = 0, mul_index = 0;

    pthread_mutex_t m1_mutex, m2_mutex, mul_mutex;
    pthread_mutex_init(&m1_mutex, nullptr);
    pthread_mutex_init(&m2_mutex, nullptr);
    pthread_mutex_init(&mul_mutex, nullptr);

    for (size_t i = 0; i < NUM_CORES / 2; i++) {
        fill_data[i].m = A;
        fill_data[i].index = &m1_index;
        fill_data[i].mutex = &m1_mutex;
        pthread_create(&tid[i], nullptr, matrixFill, &fill_data[i]);
    }

    for (size_t i = NUM_CORES / 2; i < NUM_CORES; i++) {
        fill_data[i].m = B;
        fill_data[i].index = &m2_index;
        fill_data[i].mutex = &m2_mutex;
        pthread_create(&tid[i], nullptr, matrixFill, &fill_data[i]);
    }

    for (size_t i = 0; i < NUM_CORES; i++) {
        pthread_join(tid[i], nullptr);
    } 

    for (size_t i = 0; i < NUM_CORES; i++) {
        dot_p_data[i].m1 = A;
        dot_p_data[i].m2 = B;
        dot_p_data[i].m3 = C;
        dot_p_data[i].index = &mul_index;
        dot_p_data[i].mutex = &mul_mutex;
        pthread_create(&tid[i], nullptr, matrixDotP, &dot_p_data[i]);
    }

    for (size_t i = 0; i < NUM_CORES; i++) {
        pthread_join(tid[i], nullptr);
    } 

    // Saves the finish time.
    auto finish = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(finish - start);

    cout << endl
        << "Parallel Runtime (\xC2\xB5s):            " // UTF-8 for "micro".
        << duration.count() << " microseconds" 
        << endl << endl;

    matrixFile(A, "PTHREADS_matrixA.mtrx");
    matrixFile(B, "PTHREADS_matrixB.mtrx");
    matrixFile(C, "PTHREADS_matrixC.mtrx");

    // Deleting allocated memory.
    for (int i = 0; i < N; ++i) {
        delete[] A[i]; 
        delete[] B[i]; 
        delete[] C[i]; 
    } 

    delete[] fill_data;
    delete[] dot_p_data;
    pthread_mutex_destroy(&m1_mutex);
    pthread_mutex_destroy(&m2_mutex);
    pthread_mutex_destroy(&mul_mutex);

    return EXIT_SUCCESS;
}
