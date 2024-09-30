#include <iostream>
#include <time.h> 
#include <chrono>
#include <unistd.h>
#include <algorithm>
#include <iomanip>
#include <omp.h>

using namespace std::chrono;
using namespace std;

const long NUM_CORES = sysconf(_SC_NPROCESSORS_ONLN);
const size_t VECTOR_SIZE = 1000000;
const size_t PARTITION_SIZE = VECTOR_SIZE / 50;

// Fills a vector with random integer entries.
void randomVector(int vector[], size_t start, size_t end) {
    for (size_t i = start; i < end; i++) {
        // Initialises vector entry to random number in [0, 100].
        vector[i] = rand() % 100;
    }
}

int main() {
    // Sets the random seed to a different value each time the program executes.
    // Time(0) returns the current time since Epoch time (Jan 1 1970).
    srand(time(0));
    int *v1, *v2, *v3;

    // Saves the start time.
    auto omp_sched_start = high_resolution_clock::now();

    // Allocating memory for each vector with space for "size" entries.
    v1 = (int *) malloc(VECTOR_SIZE * sizeof(int));
    v2 = (int *) malloc(VECTOR_SIZE * sizeof(int));
    v3 = (int *) malloc(VECTOR_SIZE * sizeof(int));

    // // schedule() values were changed for each compilation. ie the executable
    // // dynam_1 had schedule(dynamic, 1).
    // #pragma omp parallel for default(none) shared(v1) schedule(static, 20)
    // for (size_t i = 0; i < VECTOR_SIZE; i++) {
    //     v1[i] = rand() % 100;
    // }

    // #pragma omp parallel for default(none) shared(v2) schedule(static, 20)
    // for (size_t i = 0; i < VECTOR_SIZE; i++) {
    //     v2[i] = rand() % 100;
    // }

    randomVector(v1, 0, VECTOR_SIZE);
    randomVector(v2, 0, VECTOR_SIZE);

    #pragma omp parallel for default(none) shared(v1, v2, v3) schedule(static, 20)
    for (size_t i = 0; i <  VECTOR_SIZE; i++) {
        v3[i] = v1[i] + v2[i];
    }

    // Saves the end time.
    auto omp_sched_stop = high_resolution_clock::now();

    // Freeing/destroying all resources created.  
    free(v1);
    free(v2);
    free(v3);

    auto time = duration_cast<microseconds>(omp_sched_stop - omp_sched_start);
    cout << time.count();

    return 0;
}