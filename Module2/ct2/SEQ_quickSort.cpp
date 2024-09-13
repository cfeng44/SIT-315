// ----------------------------------------------------------------------------
// File:        SEQ_quickSort.cpp
// Author:      Codey Funston
// Version:     1.0.0 
//
// Description: 
//             
//              Sequential implementation of the quick sort algorithm.
//
// ----------------------------------------------------------------------------

#include <chrono>
#include <iostream>
#include <time.h> 
#include <chrono>
#include <vector>

using namespace std::chrono;
using namespace std;

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
    quickSort(data, start, l_pos - 1);
    quickSort(data, l_pos + 1, end);
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

    vector<int> data = vector<int>(2000000);
    for (size_t i = 0; i < 100; i++) {
        data[i] = rand() % 100;
    }

    // printVector(data, "Unsorted");
    quickSort(data, 0, data.size() - 1);
    // printVector(data, "Sorted");

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