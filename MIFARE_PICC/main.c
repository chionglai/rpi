/*
 * main.c
 *
 *  Created on: 08/08/2013
 *      Author: lcc
 *
 *  The default 6-byte key is xFF xFF xFF xFF xFF xFF
 *  The 4-byte serial is the first 4 bytes of card's UID, obtained from
 *  running mifare_activate(). Each authentication is only for 4 address block.
 *  E.g. authenticate for any of the block address 0-3 will have same effect and
 *  only allows operation on block address 0-3. To operate outside these block
 *  address, need to re-authenticate for the desired new address.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "util/status.h"
#include "util/std.h"
#include "module/mfrc522-rfid/mfrc522.h"
#include "module/mfrc522-rfid/picc14443a3.h"

#include "3p-lib/bcm2835-1.50/inc/bcm2835.h"

#define MAX_CHAR	128
#define DELIMITER	" "

#define MFRC522_SPI_INDEX	(0)
#define MFRC522_CS_INDEX	(0)

#if 0
/* Deprecated. Merged into runApp(). */

void runActivate(mfrc522_t *mfrc522) {
	uint8_t status;
	uint8_t uid[10];
	uint8_t sak;
	uint16_t atqa;
	int i;
	uint8_t key[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	status = picc14443a3_activate(mfrc522, uid, &sak, &atqa);

	fprintf(stdout, "status = %02xh | sak = %02xh | uid = ", status, sak);
	for (i = 0; i < 10; i++) {
		fprintf(stdout, "%02xh ", uid[i]);
	}
	fprintf(stdout, "\n");

	status = mifare_getType(sak);

	fprintf(stdout, "type = %02xh\n", status);

	status = mfrc522_authMifare(mfrc522, MFRC522_MIFARE_KEY_A, 0x04, key, uid);

	fprintf(stdout, "auth = %02xh\n", status);
}


void runRead(mfrc522_t *mfrc522) {
	char input[MAX_CHAR];
	uint8_t data[16];
	uint8_t addr;
	unsigned int temp;
	uint8_t status;
	int i;

	while (1) {
		do {
			std_in_readLine(input, MAX_CHAR, "Enter block address (00h - 3Fh) > ");
		} while (sscanf(input, "%d", &temp) == 0 && sscanf(input, "x%x", &temp) == 0);

		addr = (uint8_t)temp & 0xFF;
		status = picc14443a3_read(mfrc522, addr, data);

		fprintf(stdout, "status = %02xh | data = ", status);
		for (i = 0; i < 16; i++) {
			fprintf(stdout, "%02xh ", data[i]);
		}
		fprintf(stdout, "\n");
	}
}
#endif

void printHelp(void) {
	printf("Command help:\n");
	printf("  Activate    : a\n");
	printf("  Authenticate: u type address key0 ... key5 serial0 ... serial3\n");
	printf("  Read        : r address\n");
	printf("  Dump        : p startAddr endAddr(inclusive)\n");
	printf("  Write       : w address data0 ... data15\n");
	printf("  Decrement   : d address data0 ... data3\n");
	printf("  Increment   : i address data0 ... data3\n");
	printf("  Restore     : r address\n");
	printf("  Transfer    : t \n");
	printf("  Halt        : h\n");
	printf("  Quit        : q\n");
	printf("  Print help  : ?\n");
	printf("\nThe normal operation is Activate -> Authenticate -> Perform operation -> Halt.\n");
	printf("If error occurs, always Halt the card and restart the normal operation process.\n");

}


void runApp(mfrc522_t *mfrc522) {
	char input[MAX_CHAR];
	char* tok;
	uint8_t* uid;
	uint8_t sak;
	uint16_t atqa;
	uint8_t status;
	int i, temp;
	uint8_t keyType;
	uint8_t addr;
	uint8_t endAddr;
	uint8_t key[6];
	uint8_t serial[4];
	uint8_t data[16];

	printHelp();

#if DEBUG_AUTO == 1
	uint8_t* act = "a";
	uint8_t* auth = "u b 4 xbb xbb xbb xbb xbb xbb xed xf4 x6e x0a";
	uint8_t* write = "w 1 x11 x22 x33 x44 x55 x66 x77 x88 x99 xaa xbb xcc xdd xee xff x10";
	uint8_t* restore = "e 2";
	uint8_t* transfer = "t 3";
	uint8_t* increment = "i 4 x1 x2 x3 x4";
	uint8_t* decrement = "d 4 x1 x2 x3 x4";
	int debug = 0;
#endif

	do {
start:

#if DEBUG_AUTO == 1
		if (debug == 0) {
			input[0] = act[0];
		} else if (debug == 1) {
			i = 0;
			while ((input[i] = auth[i++]) != '\0');
		} else if (debug == 2){
			i = 0;
			while ((input[i] = increment[i++]) != '\0');
		} else {
			i = 0;
			while ((input[i] = transfer[i++]) != '\0');
		}
		debug++;
#else
		std_in_readLine(input, MAX_CHAR, "> ");
#endif

		tok = strtok(input, DELIMITER);

		if (tok == NULL)
			goto start;

		switch(tok[0]) {
		case 'a':
		case 'A':	// activate card
			uid = (uint8_t*) calloc((size_t) 1, (size_t) 10);
			status = picc14443a3_activate(mfrc522, uid, &sak, &atqa);

			if (status == MIFARE_OK) {
				printf("OK: ATQA = %04xh | SAK = %02xh | UID = ", atqa, sak);

				for (i = 0; i < 10; i++)
					printf("%02xh ", uid[i]);

				printf("\n");
			} else {
				printf("ERROR: mifare_activate() status: %02xh | error: %02xh\n", status, uid[0]);
			}
			break;

		case 'u':
		case 'U':	// authentication
			// get key type 'a' or 'b'
			tok = strtok(NULL, DELIMITER);
			if (tok == NULL) {
				printf("ERROR: Not enough argument, expected type of key\n");
				goto start;
			}

			if (tok[0] == 'a' || tok[0] == 'A') {
				keyType = MFRC522_MIFARE_KEY_A;
			} else if (tok[0] == 'b' || tok[0] == 'B') {
				keyType = MFRC522_MIFARE_KEY_B;
			} else {
				printf("ERROR: Key type '%c'\n", tok[0]);
				goto start;
			}

			// get sector address
			tok = strtok(NULL, DELIMITER);
			if (tok == NULL) {
				printf("ERROR: Not enough argument, expected sector address\n");
				goto start;
			} else if (sscanf(tok, "%d", &temp) == 0 && sscanf(tok, "x%x", &temp) == 0) {
				printf("ERROR: Unknown sector address '%s'\n", tok);
				goto start;
			} else {
				addr = temp & 0xFF;
			}

			// get 6-byte key
			for (i = 0; i < 6; i++) {
				tok = strtok(NULL, DELIMITER);
				if (tok == NULL) {
					printf("ERROR: Not enough argument, expected 6-byte key\n");
					goto start;
				} else if (sscanf(tok, "%d", &temp) == 0 && sscanf(tok, "x%x", &temp) == 0) {
					printf("ERROR: Unknown key '%s'\n", tok);
					goto start;
				} else {
					key[i] = temp & 0xFF;
				}
			}

			// get 4-byte serial
			for (i = 0; i < 4; i++) {
				tok = strtok(NULL, DELIMITER);
				if (tok == NULL) {
					printf("ERROR: Not enough argument, expected 4-byte UID\n");
					goto start;
				} else if (sscanf(tok, "%d", &temp) == 0 && sscanf(tok, "x%x", &temp) == 0) {
					printf("ERROR: Unknown serial number '%s'\n", tok);
					goto start;
				} else {
					serial[i] = temp & 0xFF;
				}
			}

			status = mfrc522_authMifare(mfrc522, keyType, addr, key, serial);

			if (status == MFRC522_MIFARE_AUTH_SUCCESS) {
				printf("OK:\n");
			} else {
				printf("ERROR: mfrc522_authMifare() status: %02xh\n", status);
			}

			break;

		case 'r':
		case 'R':	// read
			// get block address
			tok = strtok(NULL, DELIMITER);
			if (tok == NULL) {
				printf("ERROR: Not enough argument, expected block address\n");
				goto start;
			} else if (sscanf(tok, "%d", &temp) == 0 && sscanf(tok, "x%x", &temp) == 0) {
				printf("ERROR: Unknown block address '%s'\n", tok);
				goto start;
			} else {
				addr = temp & 0xFF;
			}

			status = picc14443a3_read(mfrc522, addr, data);

			if (status == MIFARE_OK) {
				printf("OK: ");
				for (i = 0; i < 16; i++)
					printf("%02xh ", data[i]);

				printf("\n");
			} else {
				printf("ERROR: mifare_read() status: %02xh\n", status);
			}
			break;

		case 'p':
		case 'P':
			// get block address
			tok = strtok(NULL, DELIMITER);
			if (tok == NULL) {
				printf("ERROR: Not enough argument, expected start address\n");
				goto start;
			} else if (sscanf(tok, "%d", &temp) == 0 && sscanf(tok, "x%x", &temp) == 0) {
				printf("ERROR: Unknown block address '%s'\n", tok);
				goto start;
			} else {
				addr = temp & 0xFF;
			}

			// get block address
			tok = strtok(NULL, DELIMITER);
			if (tok == NULL) {
				printf("ERROR: Not enough argument, expected end address\n");
				goto start;
			} else if (sscanf(tok, "%d", &temp) == 0 && sscanf(tok, "x%x", &temp) == 0) {
				printf("ERROR: Unknown block address '%s'\n", tok);
				goto start;
			} else {
				endAddr = temp & 0xFF;
			}

			while (addr <= endAddr) {
				status = picc14443a3_read(mfrc522, addr, data);

				if (status == MIFARE_OK) {
					printf("OK: ");
					for (i = 0; i < 16; i++)
						printf("%02xh ", data[i]);

					printf("\n");
				} else {
					printf("ERROR: mifare_read() status: %02xh\n", status);
					break;
				}
				addr++;
			}
			break;

		case 'w':
		case 'W':	// write
			// get block address
			tok = strtok(NULL, DELIMITER);
			if (tok == NULL) {
				printf("ERROR: Not enough argument, expected block address\n");
				goto start;
			} else if (sscanf(tok, "%d", &temp) == 0 && sscanf(tok, "x%x", &temp) == 0) {
				printf("ERROR: Unknown block address '%s'\n", tok);
				goto start;
			} else {
				addr = temp & 0xFF;
			}

			// get 16-byte data
			for (i = 0; i < 16; i++) {
				tok = strtok(NULL, DELIMITER);
				if (tok == NULL) {
					printf("ERROR: Not enough argument, expected 16-byte data\n");
					goto start;
				} else if (sscanf(tok, "%d", &temp) == 0 && sscanf(tok, "x%x", &temp) == 0) {
					printf("ERROR: Unknown data '%s'\n", tok);
					goto start;
				} else {
					data[i] = temp & 0xFF;
				}
			}

			status = picc14443a3_write(mfrc522, addr, data);
			if (status == MIFARE_OK) {
				printf("OK: \n");
			} else {
				printf("ERROR: mifare_write() status: %02xh\n", status);
			}
			break;

		case 'd':
		case 'D':	// decrement
			// get block address
			tok = strtok(NULL, DELIMITER);
			if (tok == NULL) {
				printf("ERROR: Not enough argument, expected block address\n");
				goto start;
			} else if (sscanf(tok, "%d", &temp) == 0 && sscanf(tok, "x%x", &temp) == 0) {
				printf("ERROR: Unknown block address '%s'\n", tok);
				goto start;
			} else {
				addr = temp & 0xFF;
			}

			// get 4-byte data
			for (i = 0; i < 4; i++) {
				tok = strtok(NULL, DELIMITER);
				if (tok == NULL) {
					printf("ERROR: Not enough argument, expected 4-byte data\n");
					goto start;
				} else if (sscanf(tok, "%d", &temp) == 0 && sscanf(tok, "x%x", &temp) == 0) {
					printf("ERROR: Unknown data '%s'\n", tok);
					goto start;
				} else {
					data[i] = temp & 0xFF;
				}
			}

			status = picc14443a3_decrement(mfrc522, addr, data);
			if (status == MIFARE_OK) {
				printf("OK: \n");
			} else {
				printf("ERROR: mifare_decrement() status: %02xh\n", status);
			}
			break;

		case 'i':
		case 'I':	// increment
			// get block address
			tok = strtok(NULL, DELIMITER);
			if (tok == NULL) {
				printf("ERROR: Not enough argument, expected block address\n");
				goto start;
			} else if (sscanf(tok, "%d", &temp) == 0 && sscanf(tok, "x%x", &temp) == 0) {
				printf("ERROR: Unknown block address '%s'\n", tok);
				goto start;
			} else {
				addr = temp & 0xFF;
			}

			// get 4-byte data
			for (i = 0; i < 4; i++) {
				tok = strtok(NULL, DELIMITER);
				if (tok == NULL) {
					printf("ERROR: Not enough argument, expected 4-byte data\n");
					goto start;
				} else if (sscanf(tok, "%d", &temp) == 0 && sscanf(tok, "x%x", &temp) == 0) {
					printf("ERROR: Unknown data '%s'\n", tok);
					goto start;
				} else {
					data[i] = temp & 0xFF;
				}
			}

			status = picc14443a3_increment(mfrc522, addr, data);
			if (status == MIFARE_OK) {
				printf("OK: \n");
			} else {
				printf("ERROR: mifare_increment() status: %02xh\n", status);
			}
			break;

		case 'e':
		case 'E':	// restore
			// get block address
			tok = strtok(NULL, DELIMITER);
			if (tok == NULL) {
				printf("ERROR: Not enough argument, expected block address\n");
				goto start;
			} else if (sscanf(tok, "%d", &temp) == 0 && sscanf(tok, "x%x", &temp) == 0) {
				printf("ERROR: Unknown block address '%s'\n", tok);
				goto start;
			} else {
				addr = temp & 0xFF;
			}

			status = picc14443a3_restore(mfrc522, addr);

			if (status == MIFARE_OK) {
				printf("OK:\n");
			} else {
				printf("ERROR: mifare_restore() status: %02xh\n", status);
			}
			break;

		case 't':
		case 'T':	// transfer
			// get block address
			tok = strtok(NULL, DELIMITER);
			if (tok == NULL) {
				printf("ERROR: Not enough argument, expected block address\n");
				goto start;
			} else if (sscanf(tok, "%d", &temp) == 0 && sscanf(tok, "x%x", &temp) == 0) {
				printf("ERROR: Unknown block address '%s'\n", tok);
				goto start;
			} else {
				addr = temp & 0xFF;
			}

			status = picc14443a3_transfer(mfrc522, addr);

			if (status == MIFARE_OK) {
				printf("OK:\n");
			} else {
				printf("ERROR: mifare_transfer() status: %02xh\n", status);
			}
			break;

		case 'h':
		case 'H':
			if (picc14443a3_deactivate(mfrc522) == MIFARE_OK) {
				printf("OK:\n");
			} else {
				printf("ERROR: NAK\n");
			}
			break;
		case 'q':
		case 'Q':
			// do nothing
			break;
		case '?':
			printHelp();
			break;

		default:
			printf("ERROR: Unknown command '%s'\n", tok);
			break;
		}
	} while(tok[0] != 'q' && tok[0] != 'Q');
}


void runBruteForce(mfrc522_t *mfrc522, long long key) {
	uint8_t addr;
//	long long key;
	uint8_t keyArr[6];
	uint8_t status;
	uint8_t uid[10];
	uint8_t sak;
	uint16_t atqa;

	addr = 0;
//	key = 0x100000000000;
	do {
		printf("\rkey = 0x%llx", key);
		fflush(stdout);

		status = picc14443a3_activate(mfrc522, uid, &sak, &atqa);

		if (status != MIFARE_OK) {
			printf("activate fails\n");
			break;
		}

		keyArr[0] = (key >> 40) & 0xFF;
		keyArr[1] = (key >> 32) & 0xFF;
		keyArr[2] = (key >> 24) & 0xFF;
		keyArr[3] = (key >> 16) & 0xFF;
		keyArr[4] = (key >> 8) & 0xFF;
		keyArr[5] = key & 0xFF;
		status = mfrc522_authMifare(mfrc522, MFRC522_MIFARE_KEY_A, addr, keyArr, uid);

		picc14443a3_deactivate(mfrc522);

		bcm2835_delay(20);
		key++;
	} while (status != MFRC522_MIFARE_AUTH_SUCCESS);

}

int main (int argc, char** argv) {
	mfrc522_t *mfrc522;
	mfrc522Cfg_t cfg;

	/** setup hardware */
	if (!bcm2835_init()) {
		return 0;
	}
	bcm2835_spi_begin();
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);

	// Max SPI data speed for MFRC522 is 10Mbit/s. bcm2835 has base clock speed of 250MHz.
	// Clock divider for bcm2835 must be even, and 250/26 = 9.61MHz
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32);

	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);

	// mfrc522 does not have CS pin.
	// it is selected based on its address, see datasheet section 8.1.2.3, pg 11
	bcm2835_spi_setChipSelectPolarity(MFRC522_CS_INDEX, LOW);

	cfg.spiIdx = MFRC522_SPI_INDEX;
	cfg.csIdx = MFRC522_CS_INDEX;
	if(mfrc522_create(&mfrc522, &cfg) != STATUS_OK) {
		return -1;
	}

	runApp(mfrc522);

#if 0
	/* Deprecated. */
	runActivate(mfrc522);
	runRead(mfrc522);
	picc14443a3_deactivate(mfrc522);
	printf("%d %s %s", argc, argv[0], argv[1]);
#endif

#if RUN_BRUTE_FORCE == 1
	/* My brute force attempt to crack mifare PICC card. */
	long long key;
	sscanf(argv[1], "%llx", &key);
	printf("key = 0x%llx\n", key);
	runBruteForce(mfrc522, key);
#endif

	return 0;
}

