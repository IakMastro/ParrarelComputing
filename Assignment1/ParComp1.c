#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int main(int argc, char *argv[]) {
    // Αρχικοποίηση τιμών
    int rank, size, tag = 100;
    MPI_Status status;

    // Αρχικοποίηση του MPI 
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    for (;;) {
        if (rank == 0) {
            int sum;

            printf("Δώσε το σύνολο των αριθμών της ακολουθίας: ");
            fflush(stdout);

            // Εάν το υπόλοιπο της διαίρεσης sum / (size - 1) μας κάνει 0, τότε σημαίνει ότι το σύνολο είναι ακέραιο πολλαπλάσιο των cores.
            for (;;) {
                scanf("%d", &sum);
                fflush(stdin);

                if (sum % size == 0)
                    break;

                printf("Πρέπει ο αριθμός να είναι ακέραιος πολλαπλάσιος του %d.\nΠαρακαλώ δωστέ τον σωστό αριθμό: ", size - 1);
                fflush(stdout);
            }
            
            int coreSum = sum / size;
            for (int i = 0; i < size; i++)
                MPI_Send(&coreSum, sizeof(int), MPI_INT, i, tag, MPI_COMM_WORLD);

            int coreToSend = 0;
            for (int i = 0; i < sum; i++) {
                // Εάν το υπόλοιο της διέρεσης του ιοστό στοίχειο της ακολουθίας με το σύνολο είναι ίσο με το σύνολο που παίρνει κάθε core,
                // τότε το στέλνει στο επόμενο core.
                if (i % sum == coreSum) coreToSend++;

                int num;
                printf("Δώσε %dιοστό αριθμό: ", i);
                fflush(stdout);
                scanf("%d", &num);
                fflush(stdin);
                MPI_Send(&num, sizeof(int), MPI_INT, coreToSend, tag, MPI_COMM_WORLD);
            }
        }
            

        char buff[100];

        int *array;
        int sum;

        MPI_Recv(&sum, sizeof(int), MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
            
        array = (int *)malloc(sizeof(int) * sum);

        for (int i = 0; i < sum; i++)  
            MPI_Recv(&array[i], sizeof(int), MPI_INT, 0, tag, MPI_COMM_WORLD, &status);

        bool correct = true;
        int wrongIndex = (rank - 1) + sum;

        // Εάν το rank δεν είναι το πρώτο, τότε στείλε την πρώτη θέση του πίνακα για να συγκρίθει με την τελευταία του προηγουμένου.
        if (rank > 0)
            MPI_Send(&array[0], sizeof(int), MPI_INT, rank - 1, tag, MPI_COMM_WORLD);

        for (int i = 0; i < sum; i++)
            if (array[i] > array[i + 1]) {
                correct = false;
                wrongIndex += i;
            }

        // Εάν το rank δεν είναι το τελευταίο, τότε σύγκρινε το τελευταίο στοιχείο του πίνακα με το πρώτο του επόμενου.
        if (rank < size - 1) {
            int num;
            MPI_Recv(&num, sizeof(int), MPI_INT, rank + 1, tag, MPI_COMM_WORLD, &status);

            if (array[sum - 1] > num) {
                correct = false;
                wrongIndex = (rank - 1) + (sum - 1);
            }
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

        if (rank == 0) {
            // Εδώ κρατάει στην μνήμη σε ποιο core βρίσκετε το λάθος στοιχείο (εάν υπάρχει) για να το χρεισημοποιήσει αργότερα.
            int wrongCore;
            for (int i = 0; i < size; i++) {
                MPI_Recv(buff, 100, MPI_CHAR, i, tag, MPI_COMM_WORLD, &status);
                if (strcmp(buff, "Όχι") == 0) {
                    wrongCore = i;
                    break;
                }
            }
            printf("Ήταν ταξινομημένη η ακουλουθία; %s\n", buff);

            if (strcmp(buff, "Όχι") == 0) {
                MPI_Recv(buff, 100, MPI_CHAR, wrongCore, tag, MPI_COMM_WORLD, &status);
                printf("Το στοίχειο που χαλάει την συμμετρία είναι στην θέση: %s\n", buff);
            }

            int answer;
            printf("Συνέχεια - 0 Διακοπή - 1: ");
            fflush(stdout);
            scanf("%d", &answer);

            for (int i = 1; i < size; i++)
                MPI_Send(&answer, sizeof(int), MPI_INT, i, tag, MPI_COMM_WORLD);

            if (answer == 1) break;
        }

        else {
            int answer;
            MPI_Recv(&answer, sizeof(int), MPI_INT, 0, tag, MPI_COMM_WORLD, &status);

            if (answer == 1) break; 
        }
    }

    MPI_Finalize();
    
    return 0;         
}