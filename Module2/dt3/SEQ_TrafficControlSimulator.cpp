// ----------------------------------------------------------------------------
// File:        SEQ_TrafficControlSimulator.cpp
// Author:      Codey Funston
// Version:     1.0.0
//
// Description: 
//             
//              A sequential implementation of a consumer/producer design.
//
// ----------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <chrono>
#include <time.h>
#include <string>
#include <unistd.h>
#include <sstream>
#include <queue>
#include <algorithm>

using namespace std::chrono;
using namespace std;

const int BUFF_SIZE = 100;
const int FINISHED = -1;

// The number of entries in the data file will be 96 * NUM_TRAFFIC_LIGHTS.
// This is why "cars" values are allowed to be up to 100,000. Otherwise
// the top N most congested traffic lights usually have the same number
// of cars passing, for small N.

const int NUM_TRAFFIC_LIGHTS = 1000;

struct TrafficLightRecord {
    // Even though 24 hr times do not behave like usual base 10 numbers, they
    // are still comparable and each consecutive minute is represented by a
    // larger integer.

    int time;
    int id;
    int cars;
};

// Comparer for use in sorting congested traffic lights.
bool compRecord(const TrafficLightRecord r_a, const TrafficLightRecord r_b) {
    return (r_a.cars < r_b.cars);
}

// Returns a single entry from the data file.
// 
// Or
//
// Returns an empty string ("") if at EOF.
string readDataFromFile(ifstream &file) {
    // By passing the file stream object in the file can be opened just once
    // in main().

    string line;

    if (getline(file, line)) {
        return line;
    }
    else {
        return "";
    }
}

// Takes a data file entry as a string and converts it to the traffic light
// struct.
TrafficLightRecord stringToRecord(string in_string) {
    TrafficLightRecord record;
    istringstream data_string_stream(in_string);

    // By using a string stream we can take each word from the string without
    // having to do manual checking for spaces by moving through character by
    // character.

    data_string_stream >> record.time;
    data_string_stream >> record.id;
    data_string_stream >> record.cars;

    return record;
}

// Reads entries from the data file and places them in the queue if there
// is space.
void produce(ifstream &data_file, queue<TrafficLightRecord> &buffer) {
    // Checks to see if the buffer is full, and if it is, waits for 
    // it to have space.
    if (buffer.size() == BUFF_SIZE) {
        return;
    }

    string str_record = readDataFromFile(data_file);

    // When all data from the file has been read.
    if (str_record.empty()) {
        // A "silly" record is used to signal to all the consumers
        // that there is no data left.
        TrafficLightRecord silly_record = {-1, -1, -1};
        buffer.push(silly_record);
        return;
    }

    TrafficLightRecord record = stringToRecord(str_record);
    buffer.push(record);
}

// Takes data from the queue, then places it in a records container.
int consume(queue<TrafficLightRecord> &buffer,
                 vector<TrafficLightRecord> &records) {
    // Checks to see if the buffer has any tasks, if not, waits for a 
    // one.
    if (buffer.empty()) {
        return 1;
    }

    TrafficLightRecord record = buffer.front();

    // If a "silly" record is received the function exits because all the
    // data from the file has been received and all tasks have been
    // assigned/completed.
    if (record.time == -1) {
        return FINISHED;
    }

    // After calling front() on a queue it must be popped because pop()
    // doesn't return the value removed unlike most other languages.
    buffer.pop();
    records.push_back(record);
    return 1;
}

// Moves program back and fourth between producing and consuming.
void run(ifstream &data_file, queue<TrafficLightRecord> &buffer,
                    vector<TrafficLightRecord> &records) {
    int status = 1;
    while (status != FINISHED) {
        produce(data_file, buffer);
        status = consume(buffer, records);
    }
}

// Returns a vector of traffic light records with the most congestion.
//
// @param N how many of the most congested lights you want data on.
// @param hr the hour of the day that you care about.
vector<TrafficLightRecord> mostCongestion(vector<TrafficLightRecord> &records,
                                            int hr_start, int N) {
    vector<TrafficLightRecord> N_most_congested_lights(N);
    vector<TrafficLightRecord> subset;

    for (int i = 0; i < records.size(); i++) {
        int hr_val = records[i].time / 100;
        if (hr_val >= hr_start && hr_val < hr_start + 1) {
            subset.push_back(records[i]);
        }
    }

    // Doing an nth_element sort is more efficient for our requirements since
    // it puts all the elements that are greater than the element at index n
    // in a sorted version of the vector. So we end up with all the elements
    // we want to the right (unsorted, they are sorted separately below).
    //
    // It runs in O(n) time as opposed to O(nlogn) time for a whole sort.
    nth_element(subset.begin(), subset.end() - N, subset.end(), compRecord);

    for (int i = subset.size() - N, j = 0; i < subset.size(), j < N; i++, j++) {
        N_most_congested_lights[j] = subset[i];
    }

    sort(N_most_congested_lights.begin(), N_most_congested_lights.end(), compRecord);

    return N_most_congested_lights;
}

// Returns formatted string representation of a traffic light record.
string visualRecord(TrafficLightRecord record) {
    string id, time, cars;

    id =   "\tID: " + to_string(record.id);
    time = "\n\tTime: " + to_string(record.time);
    cars = "\n\tCars Passed: " + to_string(record.cars);

    return id + time + cars;
}

int main(int argc, char *argv[]) {
    ifstream data_file("./data");
    queue<TrafficLightRecord> buffer;
    vector<TrafficLightRecord> records;

    run(data_file, buffer, records);

    // atoi() converts an "Array of characters TO an Int".
    int N = atoi(argv[1]), hr = atoi(argv[2]);

    vector<TrafficLightRecord> congested_lights = mostCongestion(records, hr, N);
    for (int i = congested_lights.size() - 1; i >= 0; i--) {
        cout << "(" + to_string(N - i) + ")\n" 
            << visualRecord(congested_lights[i]) << "\n\n";
    }

    data_file.close();

    return EXIT_SUCCESS;
}