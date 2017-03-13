/*
 * main.c
 *
 *  Created on: 11 Apr 2016
 *      Author: chiong
 */
#include <stdio.h>

int main (int argc, char** argv) {
	printf("Hello raspberry! I am nice?\n");
	printf("%s\n%d\n%s\n", __FILE__, __LINE__, __func__);

	return 0;
}
