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
#include <fcntl.h>

#include <ctype.h>

char buffer[1024];
int main(int argc, char *argv[]) {

	int fd = 1;
	int size = 0;

	int err;
	int mode = READ;
	char * node = argv[1];
	char * data;
	char continuous = 0;
	int tramSize = 1;
	int nonBlocking = O_NONBLOCK;
	/*Extraire les arguments : -w pour ecrire (write)
							   -r pour lire (read) en continu
							   -d pour data   (en mode ecriture)
							   -s pour le nombre de bytes a lire du fichier
							   -c pour ecrie en continu
	*/
	printf("\n");
	if(argc < 2)
		argv[0] = "-?";
	for(int i = 0; i < argc; i++){
		if(!strcmp(argv[i], "-w") || !strcmp(argv[i], "-W"))
			mode = WRITE;
		if(!strcmp(argv[i], "-r") || !strcmp(argv[i], "-R"))
			mode = READ;
		if(!strcmp(argv[i], "-d") || !strcmp(argv[i], "-D"))
			data = argv[i+1];
		if(!strcmp(argv[i], "-s") || !strcmp(argv[i], "-S"))
			tramSize = atoi(argv[i+1]);
		if(!strcmp(argv[i], "-c") || !strcmp(argv[i], "-C"))
			continuous = 1;
		if(!strcmp(argv[i], "-b") || !strcmp(argv[i], "-B"))
			nonBlocking = 0;
		if(!strcmp(argv[i], "-?") ){
			printf("usage: ./%s <noeud> {-R|-W} [-d donnnées] [-s taille] [-cb?]\n\n", PROGRAM);
			printf("\tnoeud : Emplacement du noeud. Ex.: /dev/pts/1\n");
			printf("\tR : Mode lecture  (Read) (DEFAULT)\n");
			printf("\tW : Mode ecriture (Write)\n");
			printf("\tc : Pour envoyer des données en continu\n");
			printf("\tb : Lecture/Ectriture se fait en mode bloquant\n");
			printf("\t? : Pour avoir de l'aide avec les arguments\n\n");
			printf("\td donnnées : Données a envoyer si non en continu (en string)\n");
			printf("\ts taille : Taille (en bytes) de la trame a lire\n\n");
			return 1;
		}
	}

	if(mode == WRITE){

		while(1){

			if((fd = open(node, O_WRONLY | nonBlocking)) == -1){
				printf("Cannot open Node %s.\n", node);
				return -1;
			}
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

	else if(mode == READ){
//		printf("reading %d byte trams\n", tramSize);
//		fflush(stdout);
		lseek(fd, 0, SEEK_SET);

		size = lseek(fd, 0, SEEK_END);
		lseek(fd, 0, SEEK_SET);
		printf("OK\n");
		//data = malloc(tramSize);
		while(1){
			printf("OK\n");
			if((fd = open(node, O_RDONLY | O_NONBLOCK)) == -1){
					printf("Cannot open Node %s.\n\n", node);
					return -1;
			}
			printf("Non blocking %ul\n" ,(int)O_NONBLOCK);
			//printf("%s, %d\n",buffer, 1);

			int x = read(fd, buffer, 1024);
			printf("%d\n",x);
			lseek(fd, 0, SEEK_SET);


			fflush(stdout);
			close(fd);

		}

	}

	return EXIT_SUCCESS;
}