#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char* argv[]) {
    int pid, np, elements_per_process, n_elements_received;
    MPI_Status status;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    
    if (pid == 0) { // Master process 
        printf("Hello from master process. \n");
        printf("Number of slave processes is %d\n", np - 1);

        int size, index, i;
        printf("Please enter size of array...\n");
        scanf("%d", &size);
        int arr[size];

        printf("Please enter array elements ...\n");
        for (i = 0; i < size; i++){
            scanf("%d", &arr[i]);
        }
        
        elements_per_process = size / (np - 1);
        
        // Distribute work to workers

        for (i = 0; i < np - 1; i++) {
            index =  i * elements_per_process;
            int elements_to_send = (i == np - 2 ? elements_per_process + size % (np - 1) : elements_per_process);
            MPI_Send(&elements_to_send, 1, MPI_INT, i + 1, 0, MPI_COMM_WORLD);
            MPI_Send(&arr[index], elements_to_send, MPI_INT, i + 1, 0, MPI_COMM_WORLD);
        }

        
        // Receive maxs from workers
        int tmpMax,tmpIndx;
        int max = arr[0], maxIndx = 0;
        for (i = 0; i < np - 1; i++) {
            index =  i * elements_per_process;
            MPI_Recv(&tmpMax, 1, MPI_INT, i + 1, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(&tmpIndx, 1, MPI_INT, i + 1, 0, MPI_COMM_WORLD, &status);
            if (tmpMax > max){
                max = tmpMax;
                maxIndx = index + tmpIndx;
            }
        }
        
        printf("Master process announce the final max which is %d and its index is %d.\n" , max , maxIndx);
        printf("\nThanks for using our program\n");

    } 
    else {  // Worker processes
        MPI_Recv(&n_elements_received, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        int temp[n_elements_received];  // Temporary array for workers
        MPI_Recv(&temp, n_elements_received, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        
        int partialMax = temp[0];
        int maxIndx = 0;

        for (int i = 0; i < n_elements_received; i++)
            if(temp[i] > partialMax){
                partialMax = temp[i];
                maxIndx = i;
            }

        printf("Hello from slave#%d Max number in my partition is %d and index is %i.\n", pid, partialMax, maxIndx);
        MPI_Send(&partialMax, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(&maxIndx, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}