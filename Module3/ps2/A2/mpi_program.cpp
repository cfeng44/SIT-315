#include <time.h> 
#include <chrono>
#include <mpi.h>
#include <iostream>
#include <iomanip>

using namespace std::chrono;
using namespace std;

const size_t VECTOR_SIZE = 30;
const int MASTER_RANK = 0;

void randomVector(int vector[], size_t start, size_t end) {
    for (size_t i = start; i < end; i++) {
        vector[i] = rand() % 100;
    }
}

void printVector(int *v, ostream& stream) {
    stream << "[";
    for (size_t i = 0; i < VECTOR_SIZE; i++) {
        if (i != VECTOR_SIZE -1) {
            int digit = to_string(100).length();
            stream << setw(digit) <<  v[i] << " ";
        }
        else {
            int digit = to_string(100).length();
            stream << setw(digit) <<  v[i];
            stream << "]" << endl;
        }
    }
}

int main(int argc, char *argv[]) {
    int num_tasks, rank;
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    size_t node_workload = VECTOR_SIZE / num_tasks;
    int *v1, *v2, *v3;

    int *v1_local = new int[node_workload]; 
    int *v2_local = new int[node_workload]; 
    int *v3_local = new int[node_workload];
    size_t sum_local, sum = 0;

    if (rank == MASTER_RANK) {
        auto start = high_resolution_clock::now();

        v1 = new int[VECTOR_SIZE];
        v2 = new int[VECTOR_SIZE];
        v3 = new int[VECTOR_SIZE];

        srand(time(0));
        randomVector(v1, 0, VECTOR_SIZE);
        randomVector(v2, 0, VECTOR_SIZE);

        // The master node sends and receives (it doesn't just orchestrate, it participates too) so it provides
        // the send and receive buffers.
        MPI_Scatter(v1, node_workload, MPI_INT, v1_local, node_workload, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
        MPI_Scatter(v2, node_workload, MPI_INT, v2_local, node_workload, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
        
        for (size_t i = 0; i < node_workload; i++) {
            v3_local[i] = v1_local[i] + v2_local[i];
        }

        for (size_t i = 0; i < node_workload; i++) {
            sum_local += v3_local[i];
        }

        MPI_Reduce(&sum_local, &sum, 1, MPI_INT, MPI_SUM, MASTER_RANK, MPI_COMM_WORLD);
        cout << "Total sum of v3 is: " << sum << endl;
        
        // The master node also provides the send and receive buffers here since it is taking care of
        // v3.
        MPI_Gather(v3_local, node_workload, MPI_INT, v3, node_workload, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

        auto finish = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(finish - start);

        // cout <<duration.count();

        printVector(v1, cout);
        printVector(v2, cout);
        cout << endl;
        printVector(v3, cout);

        delete[] v1;
        delete[] v2;
        delete[] v3;
    }

    else {
        // Slave nodes only receive their piece of data and use nullptr for the send buffer.
        MPI_Scatter(nullptr, node_workload, MPI_INT, v1_local, node_workload, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
        MPI_Scatter(nullptr, node_workload, MPI_INT, v2_local, node_workload, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

        for (size_t i = 0; i < node_workload; i++) {
            v3_local[i] = v1_local[i] + v2_local[i];
        }

        for (size_t i = 0; i < node_workload; i++) {
            sum_local += v3_local[i];
        }

        MPI_Reduce(&sum_local, &sum, 1, MPI_INT, MPI_SUM, MASTER_RANK, MPI_COMM_WORLD);

        // Only the master node takes care of the receiving buffer as slave nodes are just sending back their
        // small vector that they worked on.
        MPI_Gather(v3_local, node_workload, MPI_INT, nullptr, node_workload, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

        delete[] v1_local;
        delete[] v2_local;
        delete[] v3_local;
    }

    MPI_Finalize();
}