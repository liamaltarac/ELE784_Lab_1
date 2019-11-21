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

#include <unistd.h>
#include <fcntl.h>

#include <termios.h>

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}


int main(int argc, char *argv[]) {
	
	char * node = argv[1];
	int mode = argv[1];
	char k = -1;
  	int fd_r = 1;
  	  	int fd_w = 1;
	char *comm_data[1];
	int ret_val = -1 ;
	char key_val[1];
	char prev_key_val = 0;



	char *tty_name = ttyname(STDIN_FILENO);
	printf(" %s.\n\n", tty_name);
	int key_fd = open(tty_name, O_RDWR | O_NONBLOCK);
	if(key_fd == -1){
		printf("Cannot open Node %s.\n\n", node);
		return -1;
	}	
	system ("/bin/stty raw");
	printf("%s\n", node);
	
	while(key_val[0] != 27){

			fd_w = open(node, O_WRONLY | O_NONBLOCK);
			//fd_r = open(node, O_RDONLY | O_NONBLOCK);

		  	lseek(key_fd, 0, SEEK_END);
		  	read(key_fd, key_val, 1);
		  	if(key_val != 0){
		  		lseek(fd_w, 0, SEEK_END);
				write(fd_w, key_val, 1);

		  		printf("writing \n");

		  		write(key_fd, 0, 1);
		  	}

		  	//ret_val = read(fd, comm_data, 1);
		  	//printf("%d\n", ret_val);
		  	//if(!ret_val){
		  	//	printf("%c", comm_data[0]);
		  	//}

			close(fd_w);




	}

	close(key_fd);

	 system ("/bin/stty cooked");

	//close(fd);

	  return 0;




  	/*if(!strcmp(argv[2], "w") || !strcmp(argv[2], "W")){
		mode = WRITE;

		system ("/bin/stty raw");

		fd = open(node, O_WRONLY);
		while((k = getch()) != 27){
			printf("\r");
			printf("%c[2K", 27);
			printf("%d", k);
			write(fd, &k, 1);

		}
		close(fd);

 		system ("/bin/stty cooked");

  	}
	if(!strcmp(argv[2], "r") || !strcmp(argv[2], "R"))
		rl_catch_signals = 0;
        rl_bind_keyseq("\\C-g",keyPressed);
        rl_bind_keyseq("\\C-p",keyPressed);
        rl_bind_keyseq("\\C-z",keyPressed);


		mode = READ;

		while(key_val != 1){
			fd = open(node, O_RDONLY);
			read(fd, &k, 1);
			printf("%c", k);
			readline("rl> ");
		}
		close(fd); */


  	}


	

