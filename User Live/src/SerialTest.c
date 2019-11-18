/*
 ============================================================================
 Name        : SerialTest.c
 Author      : Liam
 Version     : 1
 Copyright   : Your copyright notice
 Description : Ecrit ou lit le contenu d'un noeud.
 ============================================================================
 */

#define READ 1
#define WRITE 0

#define PROGRAM "SerialTest"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <stdio.h>

#include "SerialTest.h"



int testFct(int a, int b, int c){

	printf("%d\n", a);
	printf("%d\n", b);
	printf("%d\n", c);


}

int main(int argc, char *argv[]) {


	if(mode == WRITE){
		
		if((fd = open(node, O_WRONLY)) == -1){
			printf("Cannot open Node %s.\n", node);
			return -1;
		}

		while(1){


			if(continuous){
				printf(">>>\t");
				fflush(stdout);
				scanf("%s", data);
			}
			size = strlen(data);
			printf("\tWriting %d bytes to %s\n", (int)strlen(data), node);
			err = write(fd, strcat(data, "\n"), size);
			close(fd);
			printf("\tWrite status : %d\n\n", err);
			if(!continuous){

				return err;
			}
		}
	}

}