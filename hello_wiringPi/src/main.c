/*
 * main.c
 *
 *  Created on: 23/07/2013
 *      Author: lcc
 */

#include "3p-lib/wiringPi/inc/wiringPi.h"
#include <stdio.h>

typedef struct _data_struct {
    int id;
    char* msg;
    double value;
} data_t;


void isrDoTask0(void) {
    fprintf(stdout, "ID     : 1\n");
    fprintf(stdout, "Message: This is first data\n");
    fprintf(stdout, "Value  : 10\n");
}

void isrDoTask1(void) {
    fprintf(stdout, "ID     : 2\n");
    fprintf(stdout, "Message: This is second data\n");
    fprintf(stdout, "Value  : 22\n");
}


int main (void) {
	int status;

	if ((status = wiringPiSetup()) < 0) {
		fprintf(stderr, "Error init wiringPi lib.\n");
		return status;
	}

    if ((status = wiringPiISR(4, INT_EDGE_BOTH, isrDoTask0)) < 0) {
    	fprintf(stderr, "Error register ISR 1.\n");
    	return status;
    }
    if ((status = wiringPiISR(5, INT_EDGE_BOTH, isrDoTask1)) < 0) {
    	fprintf(stderr, "Error register ISR 2.\n");
    	return status;
    }

    while (1) {
        delay(2000);
    }
}
