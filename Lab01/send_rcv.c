#include "mpi.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    char msg[20];
    int rank, tag = 100;
    int size;

    MPI_Status status;
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    switch (rank) {
        case 0:
            strcpy(msg, "Hello World!");
            MPI_Send(msg, strlen(msg) + 1, MPI_CHAR, 1, tag, MPI_COMM_WORLD);
            break;

        case 1:
            MPI_Recv(msg, 20, MPI_CHAR, 0, tag, MPI_COMM_WORLD, &status);
            printf("Proccess 1 message: %s\n", msg);
            break;
    }
        
    MPI_Finalize();
}