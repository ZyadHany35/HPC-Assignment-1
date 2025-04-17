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

	char* message = NULL; // input messege 
	int length, divide, remiander; // messege length, part length, ramining chars
	size_t buffer_size = 0; 
	int encode; // 1 to encode and 0 to decode

	if (rank == 0) {
        // this while is to choose (console/file) or keep tring again if incorrect input 
        // after choosing input the messege
        while (1) {
			char* in_type = NULL;
            printf("Choose how to input (console/file):");
            getline(&in_type, &buffer_size, stdin);
			if (strcmp("console\n",in_type) == 0) {
				printf("Enter a string: ");
		        getline(&message, &buffer_size, stdin);
                free(in_type);
				break;
			}
			else if(strcmp("file\n",in_type) == 0) {
                char* path = NULL;
                printf("Enter a file path: ");
		        getline(&path, &buffer_size, stdin);
                path[strcspn(path, "\n")] = '\0';
                FILE* file = fopen(path, "r");
                if (file == NULL) {
                    printf("coudn't open the file");
                }
                else {
		        getline(&message, &buffer_size, file);
                fclose(file);
                }    
                free(in_type);
                free(path);
				break;
			}
			else {
				printf("Enter 'console' or 'file' only\n");
			}
		}

		
        // this while is to choose (encode/decode) or keep tring again if incorrect input
		while (1) {
			printf("Enter 'en' to encode or 'de' to decode: ");
			char* en_de = NULL;
			getline(&en_de, &buffer_size, stdin);
			if (strcmp("en\n",en_de) == 0) {
				encode = 1;
                free(en_de);
				break;
			}
			else if(strcmp("de\n",en_de) == 0) {
				encode = 0;
                free(en_de);
				break;
			}
			else {
				printf("Enter 'en' or 'de' only\n");
			}
		}


        // get messege length and remove '\n'
		length = (int) strlen(message);
		if (message[length - 1] == '\n') {
			length--;
		}
        
        // calculate divided parts length
		divide = length / size;
        // if the messege length is not divisable by the size then some chars will remian so we will give them to some processes
		remiander = length % size;


		char* current = message;
		for (int i=0; i<size; i++) {
			int send_size = divide;
            // the first proccesses each will get a one char more until the remanig chars is finished
			if (i<remiander) {
				send_size++;
			}
            // send encode for other proccesses to know if we will encode o decode
			MPI_Send(&encode, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            // send the size of the messege part to be sent so that the reciving proccess know how much chars it will recive
			MPI_Send(&send_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            // send the messege part
			MPI_Send(current, send_size, MPI_CHAR, i, 0, MPI_COMM_WORLD);
			current += send_size;
		}
	}

	MPI_Recv(&encode, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	int part_size;
	MPI_Recv(&part_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	char* messege_part = malloc(part_size * sizeof(char));
	MPI_Recv(messege_part, part_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	
    // encode or decode
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
		char* final_message = malloc((length*sizeof(char))+1);
		char* current_position = final_message;
		strncpy(current_position,messege_part,part_size);
		current_position+= part_size;
		for(int i =1; i < size ; i++){
			int rec_size = divide + (i< remiander ? 1 :0 );
			
			MPI_Recv(current_position ,rec_size,MPI_CHAR,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			current_position +=  rec_size;
		}
		*current_position = '\0';


		printf("the result messege : %s \n",final_message);
		free(final_message);
		free(message);
	} else{
		MPI_Send(messege_part,part_size,MPI_CHAR,0,0,MPI_COMM_WORLD);
        free(messege_part);
	}
	MPI_Finalize();
	return 0;
}
