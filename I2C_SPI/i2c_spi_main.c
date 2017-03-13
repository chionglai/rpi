/*
 * i2c_spi_main.c
 *
 * To provide means of reading and writing to I2C and SPI from console.
 *
 *  Created on: 11/06/2013
 *      Author: lcc
 *
 *
 * Common command
 *  q
 *     - To restart this program.
 *
 * I2C command
 *  r i2c_slave_address num_of_byte
 *     - To read num_of_byte from i2c_slave_address using i2c
 *
 *  w i2c_slave_address byte0 byte1 ... byte(N-1)
 * 	   - To write N bytes of data to i2c_slave_address using i2c
 *
 * SPI command
 *  t chip_select data0 data1 ... data(N-1)
 *     - To writeN bytes from host to slave. After sending each byte, a received byte
 *       from slave to host is read.
 *     - chip_select is either 0 or 1, which corresponds to CSnA and CSnB on Gertboard
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "3p-lib/bcm2835-1.50/inc/bcm2835.h"

#define MAX_CHAR	256		// max 256 char for user input
#define STR_TOK_DELIMITER	" "

typedef enum {
	I2C = 1,
	SPI
} comm_t;

int commType = I2C;

void i2c_handler(void);
void spi_handler(void);
void close_handler(int signum);
int readLine(char* str);


int main() {
	int option;
	int result;

	signal(SIGINT, close_handler);
	signal(SIGQUIT, close_handler);
	signal(SIGABRT, close_handler);
	signal(SIGTERM, close_handler);

	if (!bcm2835_init()) {
		fprintf(stderr, "ERROR: Cannot init bcm2835!\n");
		return 1;
	}

	while(1) {
		fprintf(stdout, "Please select types of communications:\n");
		fprintf(stdout, "\t1. I2C (default)\n");
		fprintf(stdout, "\t2. SPI\n");
		fprintf(stdout, "> ");
		result = fscanf(stdin, "%d", &option);

		if (result == 1 && option >= I2C && option <= SPI)
			commType = option;

		switch (commType) {
		case I2C:
			i2c_handler();
			break;

		case SPI:
			spi_handler();
			break;
		}
		fprintf(stdout, "Program restarted...\n\n\n");
	}

	return 0;
}


void i2c_handler(void) {
	int option;
	int result;
	char input[MAX_CHAR+1];	// +1 for null terminator
	char* tok;
	char cmd;
	char i2cAddr;
	char data[MAX_CHAR];
	int dataCount;
	char errorFlag;
	unsigned int tempHex;
	int i;

	// select data rate
	fprintf(stdout, "Please select I2C data rate:\n");
	fprintf(stdout, "\t1. 100 kHz\n");
	fprintf(stdout, "\t2. 400 kHz (default)\n");
	fprintf(stdout, "\t3. 1.666 MHz\n");
	fprintf(stdout, "\t4. 1.689 MHz\n");
	fprintf(stdout, "> ");
	result = fscanf(stdin, "%d", &option);

	if (result != 1 || option < 1 || option > 4)
		option = 2;

	switch(option) {
	case 1:
		bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);
		break;
	case 3:
		bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_150);
		break;
	case 4:
		bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_148);
		break;
	default:
		bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_626);
		break;
	}

	bcm2835_i2c_begin();

	fprintf(stdout, "Command format are:\n");
	fprintf(stdout, "    Write command - w i2c_addr data0 data1 ... dataN\n");
	fprintf(stdout, "    Read command  - r i2c_addr num_byte_to_read\n");
	fprintf(stdout, "    Restart       - q\n");
	fprintf(stdout, "\nAll values can be decimal or hex (when preceded by 'x')\n");

	// getting user input
	fprintf(stdout, "> ");
	readLine(input);	// flush stdin by reading all remaining char
	result = readLine(input);

	while(result > 0 && input[0] != 'q' && input[0] != 'Q') {
		// parse command
		tok = strtok(input, STR_TOK_DELIMITER);
		cmd = tok[0];
		errorFlag = 0;
		dataCount = 0;

		// parse i2c slave address
		tok = strtok(NULL, STR_TOK_DELIMITER);
		if (tok == NULL || (1 != sscanf(tok, "x%x", &tempHex) && 1 != sscanf(tok, "%d", &tempHex))) {
			fprintf(stderr, "ERROR: Unknown/invalid I2C slave address \"%s\"\n", tok);
			errorFlag = 1;
		}
		i2cAddr = (char)tempHex;

		// parse data
		tok = strtok(NULL, STR_TOK_DELIMITER);
		while (!errorFlag && tok != NULL) {
			if (1 != sscanf(tok, "x%x", &tempHex) && 1 != sscanf(tok, "%d", &tempHex)) {
				fprintf(stderr, "ERROR: Unknown/invalid value \"%s\"\n", tok);
				errorFlag = 1;
			} else {
				data[dataCount] = (char)tempHex;
				dataCount++;
				tok = strtok(NULL, STR_TOK_DELIMITER);
			}
		}

		// only execute read/write i2c command if there is no error
		if (!errorFlag) {
			bcm2835_i2c_setSlaveAddress(i2cAddr);

			switch(cmd) {
			case 'r':	// read i2c
			case 'R':
				dataCount = data[0];
				switch(bcm2835_i2c_read(data, dataCount)) {
				case BCM2835_I2C_REASON_ERROR_NACK:
					fprintf(stderr, "ERROR: I2C NACK received\n");
					break;
				case BCM2835_I2C_REASON_ERROR_CLKT:
					fprintf(stderr, "ERROR: I2C clock stretch timeout\n");
					break;
				case BCM2835_I2C_REASON_ERROR_DATA:
					fprintf(stderr, "ERROR: I2C not all data received\n");
					break;
				default:
					fprintf(stdout, "OK: ");
					for (i = 0; i < dataCount; i++)
						fprintf(stdout, "0x%02x  ", data[i]);
					fprintf(stdout, "\n");
					break;
				}
				break;

			case 'w':	// write i2c
			case 'W':
				switch(bcm2835_i2c_write(data, dataCount)) {
				case BCM2835_I2C_REASON_ERROR_NACK:
					fprintf(stderr, "ERROR: I2C NACK received\n");
					break;
				case BCM2835_I2C_REASON_ERROR_CLKT:
					fprintf(stderr, "ERROR: I2C clock stretch timeout\n");
					break;
				case BCM2835_I2C_REASON_ERROR_DATA:
					fprintf(stderr, "ERROR: I2C not all data sent\n");
					break;
				default:
					fprintf(stdout, "OK: \n");
					break;
				}
				break;

			default:
				fprintf(stderr, "ERROR: Unknown command '%c'\n", cmd);
				break;
			}
		}

		// get user input again
		fprintf(stdout, "> ");
		result = readLine(input);
	}

	// clean-up and restart this program
	bcm2835_i2c_end();
}

void spi_handler(void) {
	int option;
	int result;
	char input[MAX_CHAR+1];	// +1 for null terminator
	char* tok;
	char cmd;
	char chipSelect;
	char data[MAX_CHAR];
	int dataCount;
	char errorFlag;
	unsigned int tempHex;
	int i;

	fprintf(stdout, "Please select data rate:\n");
	fprintf(stdout, "\t 1.   3.81 kHz\n");
	fprintf(stdout, "\t 2.   7.63 kHz\n");
	fprintf(stdout, "\t 3.  15.26 kHz\n");
	fprintf(stdout, "\t 4.  30.52 kHz\n");
	fprintf(stdout, "\t 5.  61.04 kHz\n");
	fprintf(stdout, "\t 6. 122.07 kHz\n");
	fprintf(stdout, "\t 7. 244.14 kHz\n");
	fprintf(stdout, "\t 8. 488.28 kHz\n");
	fprintf(stdout, "\t 9. 976.56 kHz\n");
	fprintf(stdout, "\t10.   1.95 MHz\n");
	fprintf(stdout, "\t11.   3.91 MHz\n");
	fprintf(stdout, "\t12.   7.81 MHz (default)\n");
	fprintf(stdout, "\t13.  15.63 MHz\n");
	fprintf(stdout, "\t14.  31.25 MHz\n");
	fprintf(stdout, "\t15.  62.50 MHz\n");
	fprintf(stdout, "\t16. 125.00 MHz\n");
	fprintf(stdout, "> ");
	result = fscanf(stdin, "%d", &option);

	if (result != 1 || option < 1 || option > 16)
		option = 12;

	switch (option) {
	case 1:
		bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_65536);
		break;
	case 2:
		bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32768);
		break;
	case 3:
		bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_16384);
		break;
	case 4:
		bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_8192);
		break;
	case 5:
		bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_4096);
		break;
	case 6:
		bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_2048);
		break;
	case 7:
		bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_1024);
		break;
	case 8:
		bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_512);
		break;
	case 9:
		bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256);
		break;
	case 10:
		bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_128);
		break;
	case 11:
		bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64);
		break;
	case 13:
		bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_16);
		break;
	case 14:
		bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_8);
		break;
	case 15:
		bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_4);
		break;
	case 16:
		bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_2);
		break;
	default:
		bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32);
		break;
	}

	fprintf(stdout, "Please select data mode:\n");
	fprintf(stdout, "\t1. Clock rest state is LOW (default)\n");
	fprintf(stdout, "\t2. Clock rest state is HIGH\n");
	fprintf(stdout, "> ");
	result = fscanf(stdin, "%d", &option);

	if (result != 1 || option < 1 || option > 2)
		option = 1;

	switch(option) {
	case 2:
		bcm2835_spi_setDataMode(BCM2835_SPI_MODE3);
		break;
	default:
		bcm2835_spi_setDataMode(BCM2835_SPI_MODE1);
		break;
	}

	fprintf(stdout, "Please select bit order:\n");
	fprintf(stdout, "\t1. MSB sent first (default)\n");
	fprintf(stdout, "\t2. LSB sent first\n");
	fprintf(stdout, "> ");
	result = fscanf(stdin, "%d", &option);

	if (result != 1 || option < 1 || option > 2)
		option = 1;

	switch (option) {
	case 2:
		bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_LSBFIRST);
		break;
	default:
		bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
		break;
	}

	fprintf(stdout, "Please select chip select polarity:\n");
	fprintf(stdout, "\t1. Active LOW (default)\n");
	fprintf(stdout, "\t2. Active HIGH\n");
	fprintf(stdout, "> ");
	result = fscanf(stdin, "%d", &option);

	if (result != 1 || option < 1 || option > 2)
		option = 1;

	switch (option) {
	case 2:
		bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, HIGH);
		bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS1, HIGH);
		bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS2, HIGH);
		break;
	default:
		bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);
		bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS1, LOW);
		bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS2, LOW);
		break;
	}
	bcm2835_spi_begin();

	fprintf(stdout, "Command format are:\n");
	fprintf(stdout, "    Transfer command - t chip_select(0 or 1) data0 data1 ... dataN\n");
	fprintf(stdout, "    Restart          - q\n");
	fprintf(stdout, "\nAll values can be decimal or hex (when preceded by 'x'). To read\n");
	fprintf(stdout, "only, data0 ... dataN becomes register addresses to be read.\n");

	// getting user input
	fprintf(stdout, "> ");
	readLine(input);	// flush stdin by reading all remaining char
	result = readLine(input);

	while(result > 0 && input[0] != 'q' && input[0] != 'Q') {
		// parse command
		tok = strtok(input, STR_TOK_DELIMITER);
		cmd = tok[0];
		errorFlag = 0;
		dataCount = 0;

		// parse i2c slave address
		tok = strtok(NULL, STR_TOK_DELIMITER);
		if (tok == NULL || 1 != sscanf(tok, "%d", &tempHex) || tempHex < 0 || tempHex > 2) {
			fprintf(stderr, "ERROR: Unknown/invalid chip select \"%s\"\n", tok);
			errorFlag = 1;
		}
		// reuse i2cAddr as chip select
		chipSelect = (char)tempHex;

		// parse data
		tok = strtok(NULL, STR_TOK_DELIMITER);
		while (!errorFlag && tok != NULL) {
			if (1 != sscanf(tok, "x%x", &tempHex) && 1 != sscanf(tok, "%d", &tempHex)) {
				fprintf(stderr, "ERROR: Unknown/invalid value \"%s\"\n", tok);
				errorFlag = 1;
			} else {
				data[dataCount] = (char)tempHex;
				dataCount++;
				tok = strtok(NULL, STR_TOK_DELIMITER);
			}
		}

		// only execute transfer spi command if there is no error
		if (!errorFlag) {
			bcm2835_spi_chipSelect(chipSelect);

			switch(cmd) {
			case 't':	// transfer spi
			case 'T':
				bcm2835_spi_transfern(data, dataCount);
				fprintf(stdout, "OK: ");
				for (i = 0; i < dataCount; i++)
					fprintf(stdout, "0x%02x  ", data[i]);
				fprintf(stdout, "\n");

				break;

			default:
				fprintf(stderr, "ERROR: Unknown command '%c'\n", cmd);
				break;
			}
		}

		// get user input again
		fprintf(stdout, "> ");
		result = readLine(input);
	}

	// clean-up and restart this program
	bcm2835_spi_end();

}

void close_handler(int signum) {
	switch(commType) {
	case I2C:
		bcm2835_i2c_end();
		break;

	case SPI:
		bcm2835_spi_end();
		break;
	}
	bcm2835_close();

	fprintf(stdout, "Program exited...\n");
	exit(signum);	// terminate program
}


int readLine(char* str) {
	int i = 0;
	char c = 0;

	while ((c = getchar()) != '\n' && c != EOF && i < MAX_CHAR) {
		str[i] = c;
		i++;
	}
	str[i] = '\0';	//todo: to be verified

	return i;
}
