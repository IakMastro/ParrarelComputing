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
    
    // Αρχικοποίηση μεταβλητών.
    int *array, *localArray;
    int sendcounts[size], displs[size];
    int sum, coreSum, min, minMaxDifference;
    float average;

    for (;;) {
        if (rank == 0) {
            printf("Δώσε το σύνολο των αριθμών της ακολουθίας: ");
            fflush(stdout);

            scanf("%d", &sum);

            // Το sendcounts έχει μέσα του το μέγεθος που παίρνει κάθε core για τον πίνακα του,
            // ενώ το displs το index κάθε μεταβλητής. 
            for (int i = 0; i < size; i++) {
                if (i < size - 1 || (sum % size) == 0) 
                    sendcounts[i] = sum / size;
                
                else
                    sendcounts[i] = sum / size + sum % size;

                if (i == 0)
                    displs[i] = 0;

                else
                    displs[i] = displs[i - 1] + sendcounts[i - 1];
            }
            fflush(stdin);
        }

        // Με MPI_Scatter στέλνουμε σε όλους τους πυρήνες το coreSum που τους ορίσαμε στο
        // sendcounts. Με MPI_Bcast στέλνουμε σε όλους τους πυρήνες το μέγεθος της ακολουθίας.
        MPI_Scatter(sendcounts, 1, MPI_INT, &coreSum, 1, MPI_INT, 0, MPI_COMM_WORLD);
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
        
        // Με το MPI_Scatterv στέλνουμε ξεχωριστά σε κάθε πυρήνα τα στοιχεία της ακολουθίας που τους 
        // ορίσαμε ότι θα πάρουν στην αρχή του προγράμματος.
        MPI_Scatterv(array, sendcounts, displs, MPI_INT, localArray, coreSum, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&average, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&min, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&minMaxDifference, 1, MPI_INT, 0, MPI_COMM_WORLD);

        // Σε αυτές τις μεταβλήτες, ψάχνουμε το σύνολο των αριθμών όπου είναι είτε μεγαλύτερο της μέσης τιμής
        // είτε μικρότερη από την μέση τιμή. Στην συνέχεια παιρνάμε όλες αυτές τις τιμές στo core 0 μέσω της MPI_Gather
        // στους πίνακες minArray και maxArray.
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

        // Πινακάς που κρατάει την τιμή (xi - m) ^ 2 με σκόπο να βρει την διασπορά.
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

        // Πινάκες για το Δ
        float *newArray; 
        float *newLocalArray;
        newArray = (float *)malloc(sizeof(float) * sum);
        newLocalArray = (float *)malloc(sizeof(float) * coreSum);
        for (int i = 0; i < coreSum ; i++) {
            newLocalArray[i] = (((float) localArray[i] - min) / minMaxDifference) * 100;
        }

        // Πινακάς που κρατάει στην θέση του το max του newLocalArray και την θέση του. 
        float localDeltaMax[2];
        localDeltaMax[0] = newLocalArray[0];
        localDeltaMax[1] = (float) (rank * coreSum);
        for (int i = 1; i < coreSum ; i++)
            if (localDeltaMax[0] < newLocalArray[i]) {
                localDeltaMax[0] = newLocalArray[i];
                localDeltaMax[1] = (float) (rank * coreSum + i);
            }

        // Με το MPI_Reduce, βρίσκει αυτόματα  το max loc του διανυσμάτος.
        float deltaMax[2];
        MPI_Reduce(localDeltaMax, deltaMax, 1, MPI_FLOAT_INT, MPI_MAXLOC, 0, MPI_COMM_WORLD);
        MPI_Gatherv(newLocalArray, coreSum, MPI_INT, newArray, sendcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            for (int i = 0; i < sum; i++) {
                printf("Δ[%d] = %f\n", i, newArray[i]);
                fflush(stdout);
            }

            printf("Ο μεγαλύτερος αριθμός του Δ είναι το %f στην θέση %f.\n", deltaMax[0], deltaMax[1]);
        }

        free(newLocalArray);

        // Με το MPI_Scan, βρίσκει αυτόματα το prefix sum για κάθε core ξεχωριστά.
        if (sum % size == 0) {
            int *product;
            product = (int *)malloc(sizeof(int) * sum);
            MPI_Scan(localArray, product, coreSum, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

            for (int i = 0; i < coreSum; i++) {
                printf("CORE %d : product[%d] = %d\n", rank, i, product[i]);
                fflush(stdout);
            }

            free(product);
        }
            
        int answer;
        if (rank == 0) {
            printf("Συνέχεια - 0 Διακοπή - 1: ");
            fflush(stdout);
            scanf("%d", &answer);
        }

        MPI_Bcast(&answer, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (answer == 1) break;
        
        free(newArray);
        free(array);
        free(localArray);
    }
    
    MPI_Finalize();
    
    return 0;
}