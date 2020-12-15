#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main( int argc, char *argv[]) {
    int rank, size, tag = 100;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    int *array, *localArray;
    int sum, coreSum, min, minMaxDifference;
    float average;

    if (rank == 0) {
        printf("Δώσε το σύνολο των αριθμών της ακολουθίας: ");
        fflush(stdout);

        for (;;) {
            scanf("%d", &sum);

            if (sum % size == 0)
                break;

            printf("Πρέπει ο αριθμός να είναι ακέραιος πολλαπλάσιος του %d.\nΠαρακαλώ δωστέ τον σωστό αριθμό: ", size);
            fflush(stdout);
        }
        fflush(stdin);

        coreSum = sum / size;
    }

    MPI_Bcast(&coreSum, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&sum, 1, MPI_INT, 0, MPI_COMM_WORLD);

    localArray  = (int *)malloc(sizeof(int) * coreSum);
    array = (int *)malloc(sizeof(int) * sum);

    if (rank == 0) {
        int numSum = 0;

        for (int i = 0; i < sum; i++) {
            printf("Δώσε %dιοστό αριθμό: ", i);
            fflush(stdout);
            scanf("%d", &array[i]);
            fflush(stdin);

            numSum += array[i];
        }

        min = array[0];
        int max = array[0];
        for (int i = 0; i < sum; i++) {
            if (array[i] > max)
                max = array[i];

            if (array[i] < min) 
                min = array[i];
        }

        average = (float) numSum / sum;
        printf("Η μέση τίμη είναι: %f\n", average);

        minMaxDifference = max - min;
        printf("Μέγιστος αριθμός διανύσματος: %d\nΜικρότερος αριθμός διανύσματος: %d\n", max, min);
        fflush(stdout);
        printf("Η διαφορά τους είναι: %d\n", minMaxDifference);
    }
    
    MPI_Scatter(array, coreSum, MPI_INT, localArray, coreSum, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&average, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&min, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&minMaxDifference, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int localMin = 0, localMax = 0;
    for (int i = 0; i < coreSum; i++) {
        if (localArray[i] < average) localMin++;
        if (localArray[i] > average) localMax++;
    }

    int *minArray;
    int *maxArray;
    minArray = (int *)malloc(sizeof(int) * size);
    maxArray = (int *)malloc(sizeof(int) * size);

    MPI_Gather(&localMin, 1, MPI_INT, minArray, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(&localMax, 1, MPI_INT, maxArray, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        int totalMin = 0, totalMax = 0;
        for (int i = 0; i < size; i++) {
            totalMin += minArray[i];
            totalMax += maxArray[i];
        }

        printf("Oι αριθμοί που είναι μεγαλύτεροι από τον μέσο όρο είναι %d, ενώ μικρότεροι είναι %d.\n", totalMax, totalMin);
        fflush(stdout);
    }

    free(minArray);
    free(maxArray);

    int *varArray;
    varArray = (int *)malloc(sizeof(int) * size);

    int varSum = 0;
    for (int i = 0; i < coreSum; i++)
        varSum += pow(localArray[i] - average, 2);

    MPI_Gather(&varSum, 1, MPI_INT, varArray, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        float var = 0;

        for (int i = 0; i < size; i++)
            var += varArray[i];

        printf("Η διασπόρα του διανύσματος είναι: %f\n", var / sum);
    }

    free(varArray);

    int *newArray; 
    int *newLocalArray;
    newArray = (int *)malloc(sizeof(int) * sum);
    newLocalArray = (int *)malloc(sizeof(int) * coreSum);
    for (int i = 0; i < coreSum ; i++) {
        newLocalArray[i] = (((float) localArray[i] - min) / minMaxDifference) * 100;
    }

    int localDeltaMax[2];
    localDeltaMax[0] = newLocalArray[0];
    localDeltaMax[1] = rank * coreSum;
    for (int i = 1; i < coreSum ; i++)
        if (localDeltaMax[0] < newLocalArray[i]) {
            localDeltaMax[0] = newLocalArray[i];
            localDeltaMax[1] = rank * coreSum + i;
        }

    int deltaMax[2];
    MPI_Reduce(localDeltaMax, deltaMax, 1, MPI_2INT, MPI_MAXLOC, 0, MPI_COMM_WORLD);
    MPI_Gather(newLocalArray, coreSum, MPI_INT, newArray, coreSum, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        for (int i = 0; i < sum; i++) {
            printf("Δ[%d] = %d\n", i, newArray[i]);
            fflush(stdout);
        }

        printf("Ο μεγαλύτερος αριθμός του Δ είναι το %d στην θέση %d.\n", deltaMax[0], deltaMax[1]);
    }

    free(newLocalArray);

    int *product;
    product = (int *)malloc(sizeof(int) * sum);
    MPI_Scan(localArray, product, coreSum, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    for (int i = 0; i < coreSum; i++) {
        printf("CORE %d : product[%d] = %d\n", rank, i, product[i]);
        fflush(stdout);
    }

    free(product);
    free(newArray);
    free(array);
    free(localArray);
    
    MPI_Finalize();
    
    return 0;
}