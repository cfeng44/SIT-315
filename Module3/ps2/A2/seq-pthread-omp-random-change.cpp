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

// For filling a vector v.
struct ThreadDataFill {
    int *v; 
    size_t *index;
    pthread_mutex_t *mutex;
};

// For adding two vectors v1 and v2 and the resultant vector vr.
struct ThreadDataAdd {
    int *v1;
    int *v2;
    int *vr; 
    size_t *index;
    pthread_mutex_t *mutex;    
};

// --- Vector Operation Functions ---

// Fills a vector with random integer entries.
void randomVector(int vector[], size_t start, size_t end) {
    for (size_t i = start; i < end; i++) {
        // Initialises vector entry to random number in [0, 100].
        vector[i] = rand() % 100;
    }
}

// --- Threading Functions ---

// Fills the section of a vector allocated to the thread, then more if it
// finishes before the whole vector is filled.
void *vectorFill(void *arg) {
    ThreadDataFill *data = static_cast<ThreadDataFill *>(arg);

    size_t start, end;
    while (true) {
        pthread_mutex_lock(data->mutex); // Index locked.
        start = *data->index;
        *data->index += PARTITION_SIZE;
        pthread_mutex_unlock(data->mutex); // Index unlocked.

        if (start >= VECTOR_SIZE) {
            // If vector is full: break.
            break;
        }
        
        // To prevent accessing an index out of range.
        end = min(start + PARTITION_SIZE, VECTOR_SIZE);
        randomVector(data->v, start, end);
    }

    pthread_exit(nullptr);
}

// Adds the sections of the vectors allocated to the thread, then more if it
// finishes before both the vectors are summed.
void *vectorAdd(void *arg) {
    ThreadDataAdd *data = static_cast<ThreadDataAdd *>(arg);

    size_t start, end;
    while (true) {
        pthread_mutex_lock(data->mutex); // Index locked.
        start = *data->index;
        *data->index += PARTITION_SIZE;
        pthread_mutex_unlock(data->mutex); // Index unlocked.

        if (start >= VECTOR_SIZE) {
            // If vector is full: break.
            break;
        }

        // To prevent accessing an index out of range.
        end = min(start + PARTITION_SIZE, VECTOR_SIZE);
         for (size_t i = start; i < end; i++){
            data->vr[i] = data->v1[i] + data->v2[i];
        }
    }

    pthread_exit(nullptr);
}

// --- Main ---

int main() {
    // Sets the random seed to a different value each time the program executes.
    // Time(0) returns the current time since Epoch time (Jan 1 1970).
    srand(time(0));
    int *v1, *v2, *v3;

    // --- Sequential Version ---
    #pragma region SEQUENTIAL

    // Saves the start time.
    auto c_start = high_resolution_clock::now();

    // Allocating memory for each vector with space for "size" entries.
    // Note: C-style memory allocation is used throughout the program so
    // that I get used to and have a better understanding of memory
    // management.
    v1 = (int *) malloc(VECTOR_SIZE * sizeof(int));
    v2 = (int *) malloc(VECTOR_SIZE * sizeof(int));
    v3 = (int *) malloc(VECTOR_SIZE * sizeof(int));

    randomVector(v1, 0, VECTOR_SIZE);
    randomVector(v2, 0, VECTOR_SIZE);

    // For each pair of entries in v1 and v2, their sum is the corresponding
    // entry in v3.
    for (size_t i = 0; i < VECTOR_SIZE; i++) {
        v3[i] = v1[i] + v2[i];
    }

    // Saves the end time.
    auto c_stop = high_resolution_clock::now();

    // Freeing/destroying all resources created.
    free(v1);
    free(v2);
    free(v3);
    #pragma endregion SEQUENTIAL

    // --- Parallelized Version ---
    #pragma region PARALLEL

    // Saves the start time.
    auto p_start = high_resolution_clock::now();

    // Allocating memory for each vector with space for "size" entries.
    v1 = (int *) malloc(VECTOR_SIZE * sizeof(int));
    v2 = (int *) malloc(VECTOR_SIZE * sizeof(int));
    v3 = (int *) malloc(VECTOR_SIZE * sizeof(int));

    // Threads and their data.
    pthread_t tid[NUM_CORES];
    ThreadDataFill *fill_data;
    ThreadDataAdd *add_data;

    fill_data = (ThreadDataFill *) malloc(NUM_CORES * sizeof(ThreadDataFill));
    add_data = (ThreadDataAdd *) malloc(NUM_CORES * sizeof(ThreadDataAdd));

    // Indicies for accessing the right parts of vectors.
    size_t v1_index = 0;
    size_t v2_index = 0;
    size_t add_index = 0;

    // Mutexes for locking access to shared indices.
    pthread_mutex_t v1_mutex;
    pthread_mutex_t v2_mutex;
    pthread_mutex_t add_mutex;

    pthread_mutex_init(&v1_mutex, nullptr);
    pthread_mutex_init(&v2_mutex, nullptr);
    pthread_mutex_init(&add_mutex, nullptr);

    // // Assigning threads for filling v1 and v2. For my machine with 11 cores
    // // v1 will be assigned 5 threads and v2 6.
    // for (size_t i = 0; i < NUM_CORES / 2; i++) {
    //     fill_data[i].v = v1;
    //     fill_data[i].index = &v1_index;
    //     fill_data[i].mutex = &v1_mutex;
    //     pthread_create(&tid[i], nullptr, vectorFill, &fill_data[i]);
    // }

    // for (size_t i = NUM_CORES / 2; i < NUM_CORES; i++) {
    //     fill_data[i].v = v2;
    //     fill_data[i].index = &v2_index;
    //     fill_data[i].mutex = &v2_mutex;
    //     pthread_create(&tid[i], nullptr, vectorFill, &fill_data[i]);
    // }

    // // Joining threads for filling v1 and v2.
    // for (size_t i = 0; i < NUM_CORES; i++) {
    //     pthread_join(tid[i], nullptr);
    // } 
    // // By this point in the program all of the threads for filling have
    // // finished executing.

    randomVector(v1, 0, VECTOR_SIZE);
    randomVector(v2, 0, VECTOR_SIZE);

    // Assigning threads for adding v1 and v2.
    for (size_t i = 0; i < NUM_CORES; i++) {
        add_data[i].v1 = v1;
        add_data[i].v2 = v2;
        add_data[i].vr = v3;
        add_data[i].index = &add_index;
        add_data[i].mutex = &add_mutex;
        pthread_create(&tid[i], nullptr, vectorAdd, &add_data[i]);
    }

    // Joining threads for adding v1 and v2.
    for (size_t i = 0; i < NUM_CORES; i++) {
        pthread_join(tid[i], nullptr);
    }
    // By this point in the program all of the threads for adding have
    // finished executing.

    // Saves the end time.
    auto p_stop = high_resolution_clock::now();

    // Freeing/destroying all resources created.
    free(v1);
    free(v2);
    free(v3);
    free(fill_data);
    free(add_data);
    pthread_mutex_destroy(&v1_mutex);
    pthread_mutex_destroy(&v2_mutex);
    pthread_mutex_destroy(&add_mutex);
    #pragma endregion PARALLEL

    // --- OpenMP Version ---
    #pragma region OMP

    // Saves the start time.
    auto omp_start = high_resolution_clock::now();

    // Allocating memory for each vector with space for "size" entries.
    v1 = (int *) malloc(VECTOR_SIZE * sizeof(int));
    v2 = (int *) malloc(VECTOR_SIZE * sizeof(int));
    v3 = (int *) malloc(VECTOR_SIZE * sizeof(int));

    // In this omp pragma and others where VECTOR_SIZE is accessed, I do not 
    // need to specify how to access it since it is declared as a constant 
    // and so no issues can arise ie race conditions with different threads 
    // accessing it. If VECTOR_SIZE was not a constant I might have used
    // firstprivate(VECTOR_SIZE) to prevent any shared use and also prevent
    // modification within the task from its original value.
    // #pragma omp parallel for default(none) shared(v1)
    // for (size_t i = 0; i < VECTOR_SIZE; i++) {
    //     v1[i] = rand() % 100;
    // }

    // #pragma omp parallel for default(none) shared(v2)
    // for (size_t i = 0; i < VECTOR_SIZE; i++) {
    //     v2[i] = rand() % 100;
    // }

    randomVector(v1, 0, VECTOR_SIZE);
    randomVector(v2, 0, VECTOR_SIZE);

    #pragma omp parallel for default(none) shared(v1, v2, v3)
    for (size_t i = 0; i <  VECTOR_SIZE; i++) {
        v3[i] = v1[i] + v2[i];
    }

    // Saves the end time.
    auto omp_stop = high_resolution_clock::now();

    // Unrelated to performance comparison (extra stuff with OMP):

    // Total sum of all elements in v3 using omp atomic.
    size_t total_atomic = 0;
    #pragma omp parallel for default(none) shared(v3, total_atomic)
    for (size_t i = 0; i < VECTOR_SIZE; i++) {
        #pragma omp atomic 
        total_atomic += v3[i];
    }

    // Total sum of all elements in v3 using omp reduction clause.
    size_t total_reduc = 0;
    #pragma omp parallel for reduction(+:total_reduc)
    for (size_t i = 0; i < VECTOR_SIZE; i++) {
        total_reduc += v3[i];
    }

    // Total sum of all elements by first summing an individual slice of v3,
    // then adding it to the shared sum.
    size_t total_crit = 0;

    #pragma omp parallel 
    // *Note: omp requires braces to be on next line for creating defined 
    // scope, errors are raised when trying to use K&R brace placement.
    {
        size_t small_sum = 0;

        #pragma omp for
        for (size_t i = 0; i < VECTOR_SIZE; i++) {
            small_sum += v3[i];
        }

        #pragma omp critical
        total_crit += small_sum;
    }

    // cout << endl
    //     << ((total_atomic == total_reduc && total_reduc == total_crit) ? "Totals are equal." : "Totals are un-equal.") << endl;

    // Freeing/destroying all resources created.    
    free(v1);
    free(v2);
    free(v3);
    #pragma endregion OMP

    // --- OpenMP Version With Scheduling Modifications ---
    #pragma region OMP_SCHED

    // Saves the start time.
    auto omp_sched_start = high_resolution_clock::now();

    // Allocating memory for each vector with space for "size" entries.
    v1 = (int *) malloc(VECTOR_SIZE * sizeof(int));
    v2 = (int *) malloc(VECTOR_SIZE * sizeof(int));
    v3 = (int *) malloc(VECTOR_SIZE * sizeof(int));

    // schedule() values were changed for each compilation. ie the executable
    // dynam_1 had schedule(dynamic, 1).
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
    #pragma endregion OMP_SCHED

    // The total time (duration) is the end time minus start time.
    auto c_duration = duration_cast<microseconds>(c_stop - c_start);
    auto p_duration = duration_cast<microseconds>(p_stop - p_start);
    auto omp_duration = duration_cast<microseconds>(omp_stop - omp_start);
    auto omp_sched_duration = duration_cast<microseconds>(omp_sched_stop - omp_sched_start);
    
    double c_to_p_factor = static_cast<double>(c_duration.count()) / static_cast<double>(p_duration.count());
    double c_to_omp_factor = static_cast<double>(c_duration.count()) / static_cast<double>(omp_duration.count());

    cout << endl
        // << "Sequential Runtime (\xC2\xB5s):          " // UTF-8 for "micro".
        // << c_duration.count() << " microseconds" 
        // << endl
        // << "Parallelized Runtime (\xC2\xB5s):        "
        << p_duration.count() //<< " microseconds"
        << endl;
        // << "OMP Runtime (\xC2\xB5s):                 "
        // << omp_duration.count() << " microseconds"
        // << endl
        // << "Parallel Speed Increase:          "
        // << setprecision(3) // (sig fig not decimal places)
        // << c_to_p_factor 
        // << endl
        // << "OMP Speed Increase:               "
        // << setprecision(3)
        // << c_to_omp_factor 
        // << endl << endl
        // << "OMP With Schedule Runtime (\xC2\xB5s):   " // UTF-8 for "micro".
        // << omp_sched_duration.count() << " microseconds" 
        // << endl;

    return EXIT_SUCCESS;
}