#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
	MPI_Init(&argc, &argv);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	char* message = NULL;
	int length, divide, remiander;
	size_t buffer_size = 0;
	int encode;
	if (rank == 0) {
		printf("Enter a string: ");
		getline(&message, &buffer_size, stdin);

		while (1) {
			printf("Enter 'en' to encode or 'de' to decode: ");
			char* en_de = NULL;
			getline(&en_de, &buffer_size, stdin);
			if (strcmp("en\n",en_de) == 0) {
				encode = 1;
				break;
			}
			else if(strcmp("de\n",en_de) == 0) {
				encode = 0;
				break;
			}
			else {
				printf("Enter 'en' or 'de' only\n");
			}
		}


		length = (int) strlen(message);
		if (message[length - 1] == '\n') {
			length--;
		}

		divide = length / size;
		remiander = length % size;

		char* current = message;
		for (int i=0; i<size; i++) {
			int send_size = divide;
			if (i<remiander) {
				send_size++;
			}

			MPI_Send(&encode, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Send(&send_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Send(current, send_size, MPI_CHAR, i, 0, MPI_COMM_WORLD);
			current += send_size;
		}
	}

	MPI_Recv(&encode, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	int part_size;
	MPI_Recv(&part_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	char messege_part[part_size];
	MPI_Recv(&messege_part, part_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	

	for (int i=0;i<part_size;i++) {
		if (encode) {
			messege_part[i]+=3;
		}
		else {
			messege_part[i]-=3;
		}
	}

	

	if (rank == 0) {
		// implement gather
		char* final_message = malloc(length+1);
		char* current_position = final_message;
		strncpy(current_position,messege_part,part_size);
		current_position+= part_size;
		for(int i =1; i < size ; i++){
			int rec_size = divide + (i< remiander ? 1 :0 );
			
			MPI_Recv(current_position ,rec_size,MPI_CHAR,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			current_position +=  rec_size;
		}
		*current_position = '\0';


		printf("the messege : %s",final_message);
		free(final_message);
		free(message);
	} else{
		MPI_Send(messege_part,part_size,MPI_CHAR,0,0,MPI_COMM_WORLD);
	}
	MPI_Finalize();
	return 0;
}
