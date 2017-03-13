/*
 * main.c
 *
 *  Created on: 22/06/2013
 *      Author: lcc
 */

#include <stdio.h>
#include <string.h>
#include "util/status.h"
#include "module/at24c32-eeprom/at24c32.h"

#include "3p-lib/bcm2835-1.50/inc/bcm2835.h"

#define AT24C32_I2C_INDEX	(0)

#define MAX_BUFFER 	(256)
#define DELIMITERS 	(" \t")


int main (void) {
	char option;
	uint32_t temp;
	uint16_t addr;
	char text[MAX_BUFFER*4];
	char* token;
	uint8_t data[MAX_BUFFER];
	int readLen;
	int i;
	at24c32_t *at24c32;
	at24c32Cfg_t cfg;

	/* Setup hardware */
	if (!bcm2835_init())
		return 0;

	bcm2835_i2c_begin();
	// 100kHz clock speed. Fastest that can be supported on 3.3Vcc, but tested to run fine on 400kHz too.
	// 400kHz clock speed. Fastest that can be supported by AT24C32 on 5Vcc
	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_626);

	cfg.i2cIdx = AT24C32_I2C_INDEX;
	if (at24c32_create(&at24c32, &cfg) != STATUS_OK)
		return 1;

	fprintf(stdout, "Option:\n");
	fprintf(stdout, "\tr - read address<xxxx> numberOfReadByte\n");
	fprintf(stdout, "\tw - write address<xxxx> data0<xx> data1<xx> ... dataN<xx>\n");
	fprintf(stdout, "\tq - quit.\n\n");

	do {
get_input:
		fprintf(stdout, "> ");
		fgets(text, MAX_BUFFER*4, stdin);

		token = strtok(text, DELIMITERS);

		if (token != NULL && sscanf(token, "%c", &option) != 1) {
			fprintf(stderr, "ERROR: Unknown command \'%s\'!\n", token);
			goto get_input;
		}

		token = strtok(NULL, DELIMITERS);
		if (token != NULL && sscanf(token, "%x", &temp) != 1) {
			fprintf(stderr, "ERROR: Unknown command \'%s\'!\n", token);
			goto get_input;
		}
		addr = (uint16_t) temp;

		token = strtok(NULL, DELIMITERS);
		switch(option) {
		case 'r':
			if (token != NULL && sscanf(token, "%d", &readLen) == 1) {

				at24c32_readRegN(at24c32, addr, data, readLen);

				for (i = 0; i < readLen; i++) {
					fprintf(stdout, "%02xh  ", data[i]);
				}
				fprintf(stdout, "\n");
			} else {
				fprintf(stderr, "ERROR: Unknown argument \'%s\'\n", text);
			}

			break;

		case 'w':
			i = 0;
			while (token != NULL) {
				if (sscanf(token, "%x", &temp)) {
					data[i] = (uint8_t) temp;
					i++;
					token = strtok (NULL, DELIMITERS);
				} else {
					fprintf(stderr, "ERROR: Unknown argument \'%s\'\n", token);
					goto get_input;
				}
			}

			at24c32_writeRegN(at24c32, addr, data, i);
			break;
		case 'q':
			at24c32_destroy(&at24c32);
			bcm2835_i2c_end();
			bcm2835_close();
			return 0;
			break;
		}
	} while (1);
}

