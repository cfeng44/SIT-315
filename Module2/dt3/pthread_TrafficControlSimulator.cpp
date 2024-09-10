// ----------------------------------------------------------------------------
// File:        pthread_TrafficControlSimulator.cpp
// Author:      Codey Funston
// Version:     .10.0
//
// Description: 
//             
//              A parallel implementation of a consumer/producer design.
//
// ----------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <chrono>
#include <time.h>
#include <pthread.h>
#include <string>
#include <unistd.h>
#include <sstream>
#include <queue>

using namespace std::chrono;
using namespace std;

const long NUM_CORES = sysconf(_SC_NPROCESSORS_ONLN);

struct TrafficLightRecord {
    // Even though 24 hr times do not behave like usual base 10 numbers, they
    // are still comparable and each consecutive minute is represented by a
    // larger integer.
    int time;
    int id;
    int cars;
};

// Data for producer threads.
struct P_ThreadData {
    pthread_mutex_t *mutex;
    pthread_cond_t *buff_has_space;
    ifstream *data_file;
    queue<TrafficLightRecord> *buffer;
};

// Data for consumer threads.
struct C_ThreadData {
    pthread_mutex_t *mutex;
    pthread_cond_t *buff_has_task;
    queue<TrafficLightRecord> *buffer;
};

void produce(ifstream &file) {
    string data = readDataFromFile(file);
    // TrafficLightRecord record = stringToRecord(data);
}

string readDataFromFile(ifstream &file) {
    string line;

    if (getline(file, line)) {
        return line;
    }
    else {
        return "";
    }
}

TrafficLightRecord stringToRecord(string in_string) {
    TrafficLightRecord record;
    istringstream data_string_stream(in_string);

    data_string_stream >> record.time;
    data_string_stream >> record.id;
    data_string_stream >> record.cars;

    return record;
}

int main() {
    ifstream myFile("./data");

    myFile.close();

    return EXIT_SUCCESS;
}