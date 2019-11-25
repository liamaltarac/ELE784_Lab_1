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
#define RW 2

#define PROGRAM "SerialTest"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#include <ctype.h>
#include "serial_driver_ioctl.h"
#include <termios.h>
char buffer[5];
int ret_val = -1 ;
char key_val[1];
int data_size;
int main(int argc, char *argv[]) {

	int fd = 1;
	int size = 0;

	int err;
	int mode = -1;
	char * node = argv[1];
	char data[1024];
	char continuous = 0;
	int tramSize = 1;
	int nonBlocking = O_NONBLOCK;
	int baud = 9600;
	int parity = 0;
	int buf_size = 0;
	/*Extraire les arguments : -w pour ecrire (write)
							   -r pour lire (read) en continu
							   -d pour data   (en mode ecriture)
							   -s pour le nombre de bytes a lire du fichier
							   -c pour ecrie en continu
	*/
	printf("\n");
	if (argc < 2)
		argv[0] = "-?";
	for (int i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "-w") || !strcmp(argv[i], "-W"))
			mode = WRITE;
		if (!strcmp(argv[i], "-r") || !strcmp(argv[i], "-R"))
			mode = READ;
		if (!strcmp(argv[i], "-rw") || !strcmp(argv[i], "-RW"))
			mode = RW;
		if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "-D"))
			strcpy(data, argv[i + 1]);
		if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "-S"))
			tramSize = atoi(argv[i + 1]);
		if (!strcmp(argv[i], "-c") || !strcmp(argv[i], "-C"))
			continuous = 1;
		//if(!strcmp(argv[i], "-b") || !strcmp(argv[i], "-B"))
		//	nonBlocking = 0;
		if (!strcmp(argv[i], "--set_parity")) {
			if ((fd = open(node, O_RDWR)) == -1) {
				printf("Cannot open Node %s.\n", node);
				return -1;
			}
			parity = atoi(argv[i + 1]);
			ioctl(fd, SERIAL_DRIVER_SET_PARITY, &parity);
			close(fd);
		}
		if (!strcmp(argv[i], "--get_buf_size")) {
			if ((fd = open(node, O_RDWR)) == -1) {
				printf("Cannot open Node %s.\n", node);
				return -1;
			}
			buf_size = argv[i + 1];
			ioctl(fd, SERIAL_DRIVER_GET_BUF_SIZE, &buf_size);
			printf("IOCTL SERIAL_DRIVER_GET_BUF_SIZE : %d \n", buf_size);
			close(fd);
		}
		if (!strcmp(argv[i], "--set_data_size")) {
			if ((fd = open(node, O_RDWR)) == -1) {
				printf("Cannot open Node %s.\n", node);
				return -1;
			}
			data_size = argv[i + 1];
			ioctl(fd, SERIAL_DRIVER_SET_DATA_SIZE, &data_size);
			printf("IOCTL SERIAL_DRIVER_SET_DATA_SIZE Done");
			close(fd);
		}
		if (!strcmp(argv[i], "--set_buf_size")) {
			if ((fd = open(node, O_RDWR)) == -1) {
				printf("Cannot open Node %s.\n", node);
				return -1;
			}
			buf_size = atoi(argv[i + 1]);
			ioctl(fd, SERIAL_DRIVER_GET_BUF_SIZE, &buf_size);
			printf("IOCTL SERIAL_DRIVER_SET_BUF_SIZE Done \n");
			close(fd);
		}
		if (!strcmp(argv[i], "--set_baud")) {
			if ((fd = open(node, O_RDWR)) == -1) {
				printf("Cannot open Node %s.\n", node);
				return -1;
			}
			baud = atoi(argv[i + 1]);
			ioctl(fd, SERIAL_DRIVER_SET_BAUD_RATE, &baud);
			printf("IOCTL SERIAL_DRIVER_SET_BAUD_RATE at %d baud : Done \n", baud);
			close(fd);
		}
		if (!strcmp(argv[i], "-?") ) {
			printf("usage: ./%s <noeud> {-R|-W} [-d donnnées] [-s taille] [-cb?] --set_parity --get_buf_size --set_buf_size --set_data_size\n\n", PROGRAM);
			printf("\tnoeud : Emplacement du noeud. Ex.: /dev/pts/1\n");
			printf("\tR : Mode lecture  (Read) (DEFAULT)\n");
			printf("\tW : Mde ecriture (Write)\n");
			printf("\tc : Pour envoyer des données en continu\n");
			printf("\ts taille : Taille (en bytes) de la trame a lire\n");
			printf("\td donnnées : Données a envoyer si non en continu (en string)\n\n");

			
			printf("\t--set_baud : Specifie la parite du port serie (0:Impaire, 1:Paire, -1:Aucune_parite)  \n");
			printf("\t--set_parity : Specifie la parite du port serie\n");
			printf("\t--get_buf_size : Retourne la taille du tampon circulaire\n");
			printf("\t--set_buf_size : Specifie la taille du tampon circulaire\n");
			printf("\t--set_data_size : Specifie la taille des donnees (5 - 8)\n\n");
			printf("\t? : Pour avoir de l'aide avec les arguments\n");

			return 1;
		}
	}

	if (mode == WRITE) {

		while (1) {

			if ((fd = open(node, O_WRONLY )) < 0) {
				printf("Cannot open Node %s.\n", node);
				return -1;
			}

			if (continuous) {
				printf(">>>\t");
				fflush(stdout);
				scanf(" %s", data);
			}

			err = write(fd, data, strlen(data));
			close(fd);
			printf("\tWrite status : %d\n\n", err);
			if (!continuous) {

				return err;
			}
		}
	}


	if (mode == READ) {

//		printf("reading %d byte trams\n", tramSize);
//		fflush(stdout);
		//lseek(fd, 0, SEEK_SET);

		//size = lseek(fd, 0, SEEK_END);
		//lseek(fd, 0, SEEK_SET);
		//printf("OK\n");
		//data = malloc(tramSize);
		char data = 0;
		while (1) {
			//printf("OK %d, %s\n", tramSize, node);
			fd = open(node, O_RDONLY );
			if (fd < 0) {
				printf("Cannot open Node %s.\n\n", node);
				return -1;
			}
			//sleep(1);
			//printf("Message :");

			err = read(fd, buffer, tramSize);

			close(fd);

			if (!continuous) {
				printf("Message : %s\n\n", buffer);
				//fflush(stdout);
				return err;
			}


			printf("%s\n", buffer);
			fflush(stdout);

		}

	}


	if (mode == RW) {
		char command[50];
		sprintf(command, "python test_live.py %s", node);
		printf("%s\n", command);
		system(command)	;

	}


	return EXIT_SUCCESS;
}
