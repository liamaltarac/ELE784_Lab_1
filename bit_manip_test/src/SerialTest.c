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
#include <stdint.h>

#include <unistd.h>
#include <stdio.h>

#include "SerialTest.h"

int main(int argc, char *argv[]) {

	uint8_t val = 0;

	printf("%u", val | (1 << 1));


}