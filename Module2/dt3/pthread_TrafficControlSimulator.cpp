// ----------------------------------------------------------------------------
// File:        pthread_TrafficControlSimulator.cpp
// Author:      Codey Funston
// Version:     1.0.0
//
// Description: 
//             
//              A parallel implementation of a consumer/producer design using
//              the POSIX threads library (pthread.h).
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
#include <algorithm>

using namespace std::chrono;
using namespace std;

const long NUM_CORES = sysconf(_SC_NPROCESSORS_ONLN);
const int NUM_THREADS = NUM_CORES;
const int BUFF_SIZE = 100;

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

// Data for producer threads.
struct Prod_ThreadData {
    pthread_mutex_t *mutex;
    pthread_cond_t *buff_has_space;
    pthread_cond_t *buff_has_task;
    ifstream *data_file;
    queue<TrafficLightRecord> *buffer;
};

// Data for consumer threads.
struct Cons_ThreadData {
    pthread_mutex_t *mutex;
    pthread_cond_t *buff_has_task;
    pthread_cond_t *buff_has_space;
    queue<TrafficLightRecord> *buffer;
    vector<TrafficLightRecord> *records;
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

// Worker function for producer threads.
//
// Producer threads read from the data file and place it in the queue if there
// is space.
//
// *Note: The function is blocking to its thread while there is no space in
// the queue.
void *produce(void *arg) {
    Prod_ThreadData *data = static_cast<Prod_ThreadData *>(arg);

    string str_record;
    while (true) {
        pthread_mutex_lock(data->mutex);

        while (data->buffer->size() == BUFF_SIZE) {
            pthread_cond_wait(data->buff_has_space, data->mutex);
        }

        str_record = readDataFromFile(*data->data_file);

        // When all data from the file has been read.
        if (str_record.empty()) {
            // A "silly" record is used to signal to all the consumers
            // that there is no data left.
            for (int i = 0; i < NUM_THREADS - (NUM_THREADS / 2); i++) {
                TrafficLightRecord silly_record;
                silly_record.time = -1;
                silly_record.id = -1;
                silly_record.cars = -1;

                data->buffer->push(silly_record);
                pthread_cond_broadcast(data->buff_has_task);
            }

            pthread_mutex_unlock(data->mutex);
            break;
        }

        TrafficLightRecord record = stringToRecord(str_record);
        data->buffer->push(record);

        pthread_cond_broadcast(data->buff_has_task);
        pthread_mutex_unlock(data->mutex);
    }

    pthread_exit(nullptr);
}

void *consume(void *arg) {
    Cons_ThreadData *data = static_cast<Cons_ThreadData *>(arg);

    while (true) {
        pthread_mutex_lock(data->mutex);

        while (data->buffer->empty()) {
            pthread_cond_wait(data->buff_has_task, data->mutex);
        }

        TrafficLightRecord record = data->buffer->front();
        if (record.time == -1) {
            pthread_mutex_unlock(data->mutex);
            break;
        }

        data->buffer->pop();
        data->records->push_back(record);

        pthread_cond_broadcast(data->buff_has_space);
        pthread_mutex_unlock(data->mutex);
    }

    pthread_exit(nullptr);
}

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

    nth_element(subset.begin(), subset.end() - N, subset.end(), compRecord);

    for (int i = subset.size() - N, j = 0; i < subset.size(), j < N; i++, j++) {
        N_most_congested_lights[j] = subset[i];
    }

    sort(N_most_congested_lights.begin(), N_most_congested_lights.end(), compRecord);

    return N_most_congested_lights;
}

string visualRecord(TrafficLightRecord record) {
    string id, time, cars;

    id =   "\tID: " + to_string(record.id);
    time = "\n\tTime: " + to_string(record.time);
    cars = "\n\tCars Passed: " + to_string(record.cars);

    return id + time + cars;
}

int main(int argc, char *argv[]) {
    ifstream myFile("./data");
    if (!myFile.is_open()) return -1;
    queue<TrafficLightRecord> buffer;

    pthread_cond_t buff_has_task, buff_has_space;
    pthread_cond_init(&buff_has_task, nullptr);
    pthread_cond_init(&buff_has_space, nullptr);

    pthread_mutex_t m;
    pthread_mutex_init(&m, nullptr);

    vector<pthread_t> tid(NUM_THREADS);
    vector<Prod_ThreadData> prod_thread_data(NUM_THREADS / 2);
    vector<Cons_ThreadData> cons_thread_data(NUM_THREADS - (NUM_THREADS / 2));
    vector<TrafficLightRecord> records;

    for (int i = 0; i < NUM_THREADS / 2; i++) {
        prod_thread_data[i].buff_has_task = &buff_has_task;
        prod_thread_data[i].buff_has_space = &buff_has_space;
        prod_thread_data[i].buffer = &buffer;
        prod_thread_data[i].mutex = &m;
        prod_thread_data[i].data_file = &myFile;

        pthread_create(&tid[i], nullptr, produce, &prod_thread_data[i]);
    }

    for (int i = NUM_THREADS / 2; i < NUM_THREADS; i++) {
        int j = i - NUM_THREADS / 2;
        cons_thread_data[j].buff_has_task = &buff_has_task;
        cons_thread_data[j].buff_has_space = &buff_has_space;
        cons_thread_data[j].buffer = &buffer;
        cons_thread_data[j].mutex = &m;
        cons_thread_data[j].records = &records;

        pthread_create(&tid[i], nullptr, consume, &cons_thread_data[j]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(tid[i], nullptr);
    }

    int N = atoi(argv[1]), hr = atoi(argv[2]);

    vector<TrafficLightRecord> congested_lights = mostCongestion(records, hr, N);
    for (int i = congested_lights.size() - 1; i >= 0; i--) {
        cout << "(" + to_string(N - i) + ")\n" 
            << visualRecord(congested_lights[i]) << "\n\n";
    }

    myFile.close();

    return EXIT_SUCCESS;
}