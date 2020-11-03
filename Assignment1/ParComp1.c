#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int main( int argc, char *argv[]) {
    int rank, tag = 100;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    int *array;

    if (rank == 0) {
        char buff[100];

        FILE *file = fopen("input.txt", "r");
        if (file == NULL) {
            printf("Δεν μπόρεσα να ανοίξω το αρχείο...\n");
            exit(-1);
        }

        fscanf(file, "%s", buff);
        MPI_Send(buff, strlen(buff) + 1, MPI_CHAR, 1, tag, MPI_COMM_WORLD);

        for (int i = 0; i < atoi(buff); i++) { 
            fscanf(file, "%s", buff);
            MPI_Send(buff, strlen(buff) + 1, MPI_CHAR, 1, tag, MPI_COMM_WORLD);
        }

        fclose(file);

        MPI_Recv(buff, 100, MPI_CHAR, 1, tag, MPI_COMM_WORLD, &status);
        printf("Ήταν ταξινομημένη η ακουλουθία; %s\n", buff);

        if (strcmp(buff, "Όχι") == 0) {
            MPI_Recv(buff, 100, MPI_CHAR, 1, tag, MPI_COMM_WORLD, &status);
            printf("Το στοίχειο που χαλάει την συμμετρία είναι στην θέση: %s\n", buff);
        }
    }
        
    if (rank == 1) {
        char buff[100];
        int *array;

        MPI_Recv(buff, 4, MPI_CHAR, 0, tag, MPI_COMM_WORLD, &status);
        
        int size = atoi(buff);
        printf("Πήρα μέγεθος: %d\n", size);

        array = (int *)malloc(sizeof(int) * size);

        for (int i = 0; i < size; i++){ 
            MPI_Recv(buff, 4, MPI_CHAR, 0, tag, MPI_COMM_WORLD, &status);
            array[i] = atoi(buff);
            printf("array[%d] = %d\n", i, array[i]);
        }

        bool correct = true;
        int wrongIndex;
        for (int i = 0; i < size; i++)
            if (array[i] > array[i + 1]) {
                correct = false;
                wrongIndex = i;
            }
        
        if (correct == true) {
            strcpy(buff, "Ναι");
            MPI_Send(buff, strlen(buff) + 1, MPI_CHAR, 0, tag, MPI_COMM_WORLD);
        } else {
            strcpy(buff, "Όχι");
            MPI_Send(buff, strlen(buff) + 1, MPI_CHAR, 0, tag, MPI_COMM_WORLD);
            sprintf(buff, "%d", wrongIndex);
            MPI_Send(buff, strlen(buff) + 1, MPI_CHAR, 0, tag, MPI_COMM_WORLD);
        }

        free(array);
    }

    MPI_Finalize();
    
    return 0;
}