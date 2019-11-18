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

#include <stdint.h>
#include <stdio.h>

#include "SerialTest.h"

#define R 1
#define W 2
#define RW R|W




int main(int argc, char *argv[]) {

	uint8_t c = RW;
	printf("%d, %d, %d\n", c, RW, c & RW);
	if(c == RW)
		printf("Equal\n");



}
