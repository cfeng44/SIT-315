#include <mpi.h>
#include <iostream>

using namespace std;

const int MASTER_RANK = 0;
const int HELLO_MESSAGE = 1;

int main(int argc, char *argv[]) {
    int num_tasks, rank, name_len;
    char name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);

    char message_to[12] = "Hello World";
    char message_from[12];
    MPI_Status status;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == MASTER_RANK) {
        for (int i = 1; i < num_tasks; i++) {
            // Sending it to processors in MPI_COMM_WORLD which includes them all.
            MPI_Send(message_to, 12, MPI_CHAR, i, HELLO_MESSAGE, MPI_COMM_WORLD);
        }

        // MPI_Bcast(message_to, 12, MPI_CHAR, MASTER_RANK, MPI_COMM_WORLD);
    }
    else {
        MPI_Get_processor_name(name, &name_len);
        MPI_Recv(message_from, 12, MPI_CHAR, MASTER_RANK, HELLO_MESSAGE, MPI_COMM_WORLD, &status);

        if (status.MPI_ERROR == 0) {
        printf("You got this message from %s, rank %d out of %d\n", message_from, rank, num_tasks);
        }
        
        else {
            printf("MPI_Recv Error...");
        }
    }

    MPI_Finalize();
}
