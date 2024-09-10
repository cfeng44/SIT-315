// ----------------------------------------------------------------------------
// File:        OMP_quickSort.cpp
// Author:      Codey Funston
// Version:     1.0.0 
//
// Description: 
//             
//              Parallel implementation of the quick sort algorithm, using 
//              OpenMP compiler directives.
//
//              For recursive quickSort() calls with sufficiently large data
//              inputs a thread is assigned and multiple calls are run in 
//              parallel. However, as the piece of data being accessed gets 
//              small (in this case 1000 seemed to be appropriate based on
//              tests with total data sizes > 10,000) the thread allocation
//              overhead is too much, so it is further calls are made
//              sequentially.
//
// ----------------------------------------------------------------------------

#include <chrono>
#include <iostream>
#include <time.h> 
#include <chrono>
#include <vector>
#include <omp.h>

using namespace std::chrono;
using namespace std;

const int NUM_CORES = omp_get_num_procs();

// A number to output as the runtime that is clearly an outlier by a
// significant amount, and very perfect with all 9's.
const size_t ERROR_TIME_VALUE = 99999999999;

// Sorts a vector of integers inplace.
// @warning: Function is recursive.
void quickSort(vector<int> &data, size_t start, size_t end) {
    if (start >= end) {
        return;
    }

    int l_pos = start, r_pos = end;
    int pivot = data[rand() % (end - start) + start];

    while (l_pos <= r_pos) {
        while (l_pos <= r_pos && pivot > data[l_pos]) {
            l_pos++; // Moves through data -->
        }

        while (l_pos <= r_pos && pivot < data[r_pos]) {
            r_pos--; // Move through data <--
        }

        if (l_pos <= r_pos) {
            swap(data[l_pos], data[r_pos]);
            l_pos++;
            r_pos--;
        }
    }

    swap(data[l_pos], pivot);

    // 
    if (end - start > 1000) {
        #pragma omp parallel num_threads(NUM_CORES)
        {
            // The sections directive above has an automatic barrier at the 
            // end of the sections, but since the threads are independent there
            // is no need for waiting.
            //
            // *Note: Even though this quickSort() is done inplace and one data
            // variable is being accessed by reference, the sections accessed
            // have no crossover.

            #pragma omp sections nowait
            {
                #pragma omp section
                {
                    quickSort(data, start, l_pos - 1);
                }

                #pragma omp section
                {
                    quickSort(data, l_pos + 1, end);
                }
            }
        }
    }

    else {
        quickSort(data, start, l_pos - 1);
        quickSort(data, l_pos + 1, end);
    }
}

bool ordered(vector<int> data) {
    for (int i = 1; i < data.size(); i++) {
        if (data[i] >= data[i - 1]) {
            continue;
        }
        return false;
    }
    return true;
}

int main() {
    srand(time(0));

    auto start = high_resolution_clock::now();

    vector<int> data = vector<int>(100);
    for (size_t i = 0; i < 100; i++) {
        data[i] = rand() % 100;
    }

    quickSort(data, 0, data.size() - 1);

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);

    if (ordered(data)) {
        cout << duration.count() << endl;
        return EXIT_SUCCESS;
    }
    else {
        cout << ERROR_TIME_VALUE << endl;
        return EXIT_FAILURE;
    }
}
