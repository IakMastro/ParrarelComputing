#include "mpi.h"
#include <stdio.h>

int main(int argc, char **argv) {
    int rank;
    int size;
    int namelen;
    char procName[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(procName, &namelen);

    if (rank == 2)
        printf("rank %d\n", rank);
    
    else
        printf("Hello world from processor %s, rank %d out of %d processors\n", procName, rank, size);
        
    MPI_Finalize();
}