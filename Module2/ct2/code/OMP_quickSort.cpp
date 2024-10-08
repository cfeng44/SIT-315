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

// A number to output as the runtime that is clearly an outlier by a significant
//  amount, and very perfect/non-random looking with all 9's.
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

    // Based on tests 1000 works well as a stopping point for assigning threads
    // as the overhead is too significant compared to the parallel speed
    // increase.
    if (end - start > 1000) {
        // shared(data) is used to ensure that threads are working on the same
        // data since this quicksort is done inplace.
        #pragma omp task shared(data)
        {
            quickSort(data, start, l_pos - 1);
        }

        #pragma omp task shared(data)
        {
            quickSort(data, l_pos + 1, end);
        }
    }

    // For when data is small, do sequentially.
    else {
        quickSort(data, start, l_pos - 1);
        quickSort(data, l_pos + 1, end);
    }
}

// Extra function that creates a parallel region once, then calls quickSort
// which handles thread allocation within using tasks.
void parQuickSort(vector<int> &data, size_t start, size_t end) {
    #pragma omp parallel
    {
        // We only want one thread to call this.
        #pragma omp single
        quickSort(data, start, end);
    }
}

// Returns true if data is sorted in ascending order.
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

    vector<int> data = vector<int>(200000);
    for (size_t i = 0; i < 100; i++) {
        data[i] = rand() % 100;
    }

    parQuickSort(data, 0, data.size() - 1);

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
