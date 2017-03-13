/*
 * main.c
 *
 *  Created on: 19/06/2013
 *      Author: lcc
 */

#include <stdio.h>
#include <pthread.h>
#include "util/status.h"
#include "module/ds1307-rtc/ds1307.h"

#include "3p-lib/bcm2835-1.50/inc/bcm2835.h"

#define DS1307_I2C_INDEX	(0)

ds1307_t *ds1307;
int32_t haltDisplay = 0;
void* display(void* ptr);

int main (void) {
	pthread_t displayThread;
	char option;
	uint8_t buffer;
	int32_t hour, minute, second;
	int32_t result;
	ds1307Cfg_t cfg;

	/* Setup hardware */
	if (!bcm2835_init())
		return 0;

	bcm2835_i2c_begin();
	// DS1307 supports max of 100kHz I2C clock speed only.
	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);

	cfg.i2cIdx = DS1307_I2C_INDEX;
	if (ds1307_create(&ds1307, &cfg) != STATUS_OK) {
		fprintf(stderr, "ERROR: Error init DS1307 module!\n");
	}

	fprintf(stdout, "Options:\n");
	fprintf(stdout, "\tt - set time.\n");
	fprintf(stdout, "\td - set date.\n");
	fprintf(stdout, "\tw - set day of week.\n");
	fprintf(stdout, "\tf - toggle between 12-hour and 24-hour format.\n");
	fprintf(stdout, "\ts - start/stop time.\n");
	fprintf(stdout, "\tq - quit.\n\n");

	pthread_create (&displayThread, NULL, display, NULL);

	while (1) {
		option = getc(stdin);
		fprintf(stdout, "\r");
		haltDisplay = 1;
		switch(option) {
		case 't':
			fprintf(stdout, "Enter hh:mm:ss to set time. 's' to skip.\n");
			fprintf(stdout, "> ");
			result = fscanf(stdin, "%d:%d:%d", &hour, &minute, &second);
			if (result == 3) {
				ds1307_setTime(ds1307, hour, minute, second);
			}
			break;
		case 'd':
			fprintf(stdout, "Enter dd:mm:yyyy to set date. 's' to skip.\n");
			fprintf(stdout, "> ");
			result = fscanf(stdin, "%d:%d:%d", &hour, &minute, &second);
			if (result == 3) {
				ds1307_setDate(ds1307, hour, minute, second);
			}
			break;
		case 'w':
			fprintf(stdout, "Enter 1-7 to set day of week. 's' to skip.\n");
			fprintf(stdout, "> ");
			result = fscanf(stdin, "%d", &hour);
			if (result == 1) {
				ds1307_setDay(ds1307, hour);
			}
			break;
		case 'f':
			ds1307_readRegN(ds1307, DS1307_REG_HOUR, &buffer, 1);
			if (buffer & DS1307_12HR_FORMAT)
				ds1307_changeFormat(ds1307, DS1307_24HR_FORMAT);
			else
				ds1307_changeFormat(ds1307, DS1307_12HR_FORMAT);
			break;
		case 's':
			ds1307_readRegN(ds1307, DS1307_REG_SECOND, &buffer, 1);
			buffer ^= 0x80;
			ds1307_updateReg(ds1307, DS1307_REG_SECOND, DS1307_CLOCK_HALT, buffer);
			break;
		case 'q':
			ds1307_destroy(&ds1307);
			bcm2835_i2c_end();
			bcm2835_close();
			return 0;
			break;
		}

		haltDisplay = 0;
	}

	return 0;
}

void *display(void* ptr) {
	int32_t hour, minute, second;
	ds1307AmPmFlag flag;
	int32_t year, month, date;
	ds1307DayOfWeek day;
	while(1) {
		if (haltDisplay == 0) {
			ds1307_getTime(ds1307, &hour, &minute, &second, &flag);
			ds1307_getDay(ds1307, &day);
			ds1307_getDate(ds1307, &date, &month, &year);

			fprintf(stdout, "\rDate: %02d/%02d/%04d, Day %d, Time: %02d:%02d:%02d",
					date, month, year, day, hour, minute, second);

			if (flag == DS1307_12HR_AM) {
				fprintf(stdout, " AM");
			} else if (flag == DS1307_12HR_PM) {
				fprintf(stdout, " PM");
			} else {
				fprintf(stdout, "   ");
			}
			fflush(stdout);
		}

		bcm2835_delay(1000);
	}

	return NULL;
}
