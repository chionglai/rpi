/*
 * main.c
 *
 *  Created on: 09/06/2013
 *      Author: lcc
 */


#include <stdio.h>
#include <string.h>
#include "util/std.h"
#include "util/status.h"
#include "module/mfrc522-rfid/mfrc522.h"

#include "3p-lib/bcm2835-1.50/inc/bcm2835.h"

#define STR_TOK_DELIMITER 	" "
#define MAX_CHAR			(64)

#define MFRC522_SPI_INDEX	(0)
#define MFRC522_CS_INDEX	(0)

// passed
void runSelfTest(mfrc522_t *mfrc522) {
	uint8_t buf[64];
	int32_t i;

	fprintf(stdout, "Version = %xh\n", mfrc522_getVersion(mfrc522));
	fflush(stdout);

	mfrc522_selfTest(mfrc522, buf);

	for (i = 0; i < 64; i++) {
		fprintf(stdout, "%02xh, ", buf[i]);

		if ((i+1) % 8 == 0)
			fprintf(stdout, "\n");
	}
	fflush(stdout);
}

// passed
void runGenId(mfrc522_t *mfrc522) {
	int32_t i;
	uint8_t buf[25];

	for (i = 0; i < sizeof(buf); i++) {
		buf[i] = 0;
	}

	mfrc522_generateRandomID(mfrc522, buf);
	for (i = 0; i < sizeof(buf); i++) {
		fprintf(stdout, "%02xh, ", buf[i]);
	}
	fprintf(stdout, "\n");
}

// passed
void runIrq(mfrc522_t *mfrc522) {
	uint16_t status;
	uint8_t buffer[2];

	mfrc522_irq_setMode(mfrc522, MFRC522_IRQ_CMOS_INVERTED);

	mfrc522_irq_disable(mfrc522, MFRC522_IRQ_ALL);
	buffer[0] = MFRC522_REG_COM_IEN;
	buffer[1] = MFRC522_REG_DIV_IEN;
	mfrc522_readRegN(mfrc522, buffer, 2);

	mfrc522_irq_enable(mfrc522, MFRC522_IRQ_IDLE);
	buffer[0] = MFRC522_REG_COM_IEN;
	buffer[1] = MFRC522_REG_DIV_IEN;
	mfrc522_readRegN(mfrc522, buffer, 2);

	mfrc522_irq_clear(mfrc522, MFRC522_IRQ_ALL);
	status = mfrc522_irq_status(mfrc522);
	fprintf(stdout, "status = 0x%04x\n", status);

	mfrc522_irq_set(mfrc522, MFRC522_IRQ_IDLE);
	status = mfrc522_irq_status(mfrc522);
	fprintf(stdout, "status = 0x%04x\n", status);

	mfrc522_sendCmd(mfrc522, MFRC522_CMD_MEM, 0);
	status = mfrc522_irq_status(mfrc522);
	fprintf(stdout, "status = 0x%04x\n", status);
}

// passed
void runCalcCrc(mfrc522_t *mfrc522) {
	uint8_t dummyData[64];
	uint8_t status;
	uint8_t buffer[2];
	int32_t i;

	for (i = 0; i < 64; i++) {
		dummyData[i] = i;
	}

	mfrc522_crc_setPreset(mfrc522, MFRC522_CRC_PRESET_6363H);

	status = mfrc522_crc_calc(mfrc522, dummyData, 64, buffer);
	if (status == MFRC522_CRC_SUCCESS) {
		fprintf(stdout, "CRC 64 OK: %02xh  %02xh\n", buffer[0], buffer[1]);
	} else {
		fprintf(stdout, "CRC 64 timeout!\n");
	}

	status = mfrc522_crc_calc(mfrc522, dummyData, 64, buffer);
	if (status == MFRC522_CRC_SUCCESS) {
		fprintf(stdout, "Same CRC 64 OK: %02xh  %02xh\n", buffer[0], buffer[1]);
	} else {
		fprintf(stdout, "Same CRC 64 timeout!\n");
	}

	status = mfrc522_crc_calc(mfrc522, &dummyData[10], 10, buffer);
	if (status == MFRC522_CRC_SUCCESS) {
		fprintf(stdout, "CRC 10 OK: %02xh  %02xh\n", buffer[0], buffer[1]);
	} else {
		fprintf(stdout, "CRC 10 timeout!\n");
	}
}

#if 0
/* These tests seems not to work completely. Use runApp() in MIFARE_PICC project
 * instead, since it contains proper activate, authenticate, read, write, etc
 * functionalities and it works. */

void runTransceive(mfrc522_t *mfrc522) {
	char input[MAX_CHAR];
	char* tok;
	uint8_t hasError;
	uint8_t data[64];
	int32_t i;
	uint32_t tempHex;
	uint8_t status;
	int txBits;

	while(1) {
		// get number of Tx bit from user
		do {
			hasError = 0;
			std_in_readInt(&txBits, "Number of Tx bits> ");
//			fprintf(stdout, "Number of Tx bits> ");
//
//			if (1 != fscanf(stdin, "%d", &txBits)) {
//				hasError = 1;
//				fprintf(stdout, "ERROR: Unknown value: %c\n", txBits);
//			}
		} while (hasError == 1);

		// read Tx data from user
		do {
			hasError = 0;
			std_in_readLine(input, MAX_CHAR, "Tx data> ");

			i = 0;
			tok = strtok(input, STR_TOK_DELIMITER);
			while (i < 64 && tok != NULL && hasError == 0) {
				if ((1 != sscanf(tok, "%d", &tempHex) && 1 != sscanf(tok, "x%x", &tempHex))) {
					hasError = 1;
					fprintf(stdout, "ERROR: Unknown input: %s\n", tok);
				} else {
					data[i] = (uint8_t)tempHex;
					i++;
					tok = strtok(NULL, STR_TOK_DELIMITER);
				}
			}
		} while (hasError == 1);


//		data[0] = 0x26;
//		txBits = 7;
		status = mfrc522_transceive(mfrc522, data, txBits, &tempHex);

//fprintf(stderr, "status = 0x%02x | error = 0x%02x\n", status, data[0]);

		switch(status) {
		case MFRC522_TXRX_ERROR:
			fprintf(stdout, "ERROR: ErrorReg =  0x%02x\n", data[0]);
			break;
		case MFRC522_TXRX_TIMEOUT:
			fprintf(stdout, "Timeout error!\n");
			break;
		case MFRC522_TXRX_OK:
			fprintf(stdout, "OK: %d | Data: ", tempHex);

			if ((tempHex & 0x07) != 0)
				tempHex += 8;

			tempHex = tempHex >> 3;
			for (i = 0; i < tempHex; i ++) {
				fprintf(stdout, "0x%02x  ", data[i]);
			}
			fprintf(stdout, "\n");
			break;
		default:
			fprintf(stdout, "Unknown error!\n");
			break;
		}

		fflush(stdout);
	}
}

// runAuthMifare() and testAuthMifare() not working. need to perform
// at least one command before can authenticate for another sector
void runAuthMifare(mfrc522_t *mfrc522) {
	uint8_t type;
	uint8_t addr;
	uint8_t key[6];
	uint8_t serial[4];
	char input[MAX_CHAR];
	char* tok;
	uint8_t hasError;
	uint8_t status;
	int32_t i;
	uint32_t tempHex;
	uint8_t uid[10];
	uint8_t sak;

	while (1) {
		// get key type from user
		do {
			std_in_readLine(input, MAX_CHAR, "Key type 'A' or 'B'> ");
			type = input[0];

			if (type == 'a' || type == 'A') {
				hasError = 0;
				type = MFRC522_MIFARE_KEY_A;
			} else if (type == 'b' || type == 'B') {
				hasError = 0;
				type = MFRC522_MIFARE_KEY_B;
			} else {
				hasError = 1;
				fprintf(stdout, "ERROR: Unknown key type: %c\n", type);
			}
		} while (hasError == 1);

		// get block address from user
		do {
			std_in_readLine(input, MAX_CHAR, "Block address> ");

			if (1 == sscanf(input, "%d", &tempHex) || 1 == sscanf(input, "x%x", &tempHex)) {
				hasError = 0;
				addr = (uint8_t)tempHex;
			} else {
				hasError = 1;
				fprintf(stdout, "ERROR: Unknown block address: %s\n", input);
			}
		} while (hasError == 1);

		// get key from user
		// For the blue keychain tag and the white card that I have, the default key is
		//		xFF xFF xFF xFF xFF xFF
		do {
			hasError = 0;
			std_in_readLine(input, MAX_CHAR, "6-byte key> ");

			for (i = 0, tok = strtok(input, STR_TOK_DELIMITER);
					(i < 6) && (hasError == 0);
					i++, tok = strtok(NULL, STR_TOK_DELIMITER)) {
				if (tok == NULL || (1 != sscanf(tok, "%d", &tempHex) && 1 != sscanf(tok, "x%x", &tempHex))) {
					hasError = 1;
					fprintf(stdout, "ERROR: Unknown key: %s\n", tok);
				} else {
					key[i] = (uint8_t)tempHex;
				}
			}
		} while (hasError == 1);

		// get card's serial number from user. The serial is the first 4 bytes of
		// the card UID. Need to cal mifare_activate() to get its UID.
		do {
			hasError = 0;
			std_in_readLine(input, MAX_CHAR, "4-byte card serial> ");

			for (i = 0, tok = strtok(input, STR_TOK_DELIMITER);
					i < 4 && hasError == 0;
					i++, tok = strtok(NULL, STR_TOK_DELIMITER)) {

				if (tok == NULL || (1 != sscanf(tok, "%d", &tempHex) && 1 != sscanf(tok, "x%x", &tempHex))) {
					hasError = 1;
					fprintf(stdout, "ERROR: Unknown serial number: %s\n", tok);
				} else {
					serial[i] = (uint8_t)tempHex;
				}
			}
		} while (hasError == 1);

		// start Mifare authentication
		status = mfrc522_authMifare(mfrc522, type, addr, key, serial);

		switch(status) {
		case MFRC522_MIFARE_AUTH_SUCCESS:
			fprintf(stdout, "OK\n");
			break;
		case MFRC522_MIFARE_AUTH_TIMEOUT:
			fprintf(stdout, "Timeout/Fail\n");
			break;
		case MFRC522_MIFARE_PROTOCOL_ERROR:
			fprintf(stdout, "Protocol error\n");
			break;
		default:
			fprintf(stdout, "Unknown error\n");
			break;
		}

	}
}

// only for 4-bytes uid
void testAuthMifare(mfrc522_t *mfrc522) {
	uint8_t buffer[10];
	uint8_t key[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	uint8_t serial[4];
	uint32_t txLen, rxLen;
	uint8_t blockAddr;
	int32_t i;
	uint8_t status;

	buffer[0] = 0x26;
	txLen = 7;
	status = mfrc522_transceive(mfrc522, buffer, txLen, &rxLen);

	if (status == MFRC522_TXRX_OK) {
		fprintf(stderr, "status = 0x%02x | bits = %d | ATQA = 0x%02x 0x%02x", status, rxLen, buffer[0], buffer[1]);
	}

	buffer[0] = 0x93;
	buffer[1] = 0x20;
	txLen = 16;
	status = mfrc522_transceive(mfrc522, buffer, txLen, &rxLen);

	if (status == MFRC522_TXRX_OK) {
		fprintf(stderr, "\nstatus = 0x%02x | bits = %d | ANTICOLL = ", status, rxLen);
		for (i = 0; i < rxLen; i += 8) {
			fprintf(stderr, "0x%02x ", buffer[i/8]);
		}
	}

	buffer[6] = buffer[4];
	for (i = 3; i >= 0; i--) {
		serial[i] = buffer[i];
		buffer[i+2] = buffer[i];
	}

	buffer[0] = 0x93;
	buffer[1] = 0x70;
	txLen = 56;

	mfrc522_updateReg(mfrc522, MFRC522_REG_TX_MODE, MFRC522_TX_CRC_EN, MFRC522_TX_CRC_EN);
	mfrc522_updateReg(mfrc522, MFRC522_REG_RX_MODE, MFRC522_RX_CRC_EN, MFRC522_RX_CRC_EN);

	status = mfrc522_transceive(mfrc522, buffer, txLen, &rxLen);

	mfrc522_updateReg(mfrc522, MFRC522_REG_TX_MODE, MFRC522_TX_CRC_EN, 0);
	mfrc522_updateReg(mfrc522, MFRC522_REG_RX_MODE, MFRC522_RX_CRC_EN, 0);

	if (status == MFRC522_TXRX_OK) {
		fprintf(stderr, "\nstatus = 0x%02x | bits = %d | ANTICOLL = ", status, rxLen);
		for (i = 0; i < rxLen; i += 8) {
			fprintf(stderr, "0x%02x ", buffer[i/8]);
		}
		fprintf(stderr, "\n");
	}

	for (blockAddr = 4; blockAddr <= 0x3F; blockAddr+=4) {
		status = mfrc522_authMifare(mfrc522, MFRC522_MIFARE_KEY_A, blockAddr, key, serial);

		fprintf(stderr, "Auth = 0x%02x\n", status);
	}
}
#endif

int main (void) {
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

	runSelfTest(mfrc522);
	runCalcCrc(mfrc522);
	runIrq(mfrc522);
	runGenId(mfrc522);

#if 0
	testAuthMifare(mfrc522);
	runAuthMifare(mfrc522);
	runTransceive(mfrc522);
#endif

	/* Clean up */
	mfrc522_destroy(&mfrc522);
	bcm2835_spi_end();
	bcm2835_close();

	return 0;
}

