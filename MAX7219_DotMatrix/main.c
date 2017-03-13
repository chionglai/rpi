/*
 * main.c
 *
 *  Created on: 11 Apr 2016
 *      Author: chiong
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include "util/status.h"
#include "util/std.h"
#include "util/buffer.h"
#include "module/max7219-dotMatrix/anim.h"
#include "module/max7219-dotMatrix/dotmatrix.h"
#include "module/max7219-dotMatrix/max7219.h"
#include "hw/spi.h"

#include "3p-lib/bcm2835-1.50/inc/bcm2835.h"


#define DOTMATRIX_SCAN_LIMIT 		(7)
#define DOTMATRIX_DECODE_MODE		(0)
#define DOTMATRIX_INTENSITY			(0xF)
#define MAX_NUM_CHAR				(257)	/* Include null terminating char. */
#define MAX_NUM_TEMP_CHAR			(8)

#define MAX7219_SPI_INDEX			(0)
#define MAX7219_CS_INDEX			(BCM2835_SPI_CS0)


typedef struct {
	max7219_t *pMax7219;
	dotMatrix_t *pDotMatrix;
} dm_t;

typedef struct {
	dm_t *pDm;
	anim_scroll_t *animScroll;
	uint8_t isRunning;
	char *str;
} scroll_t;

typedef struct {
	dm_t *pDm;
	anim_blink_t *animBlink;
	uint8_t isRunning;
	char *str;
} blink_t;

typedef struct {
	dm_t *pDm;
	anim_invert_t *animInvert;
	uint8_t isRunning;
	char *str;
} invert_t;

typedef struct {
	dm_t *pDm;
	anim_change_t *animChange;
	uint8_t isRunning;
	char **str;
	uint32_t len;
} change_t;

void close_handler(int signum);
int32_t init(dm_t* pDm, uint32_t numCascade);
void setIntensity_handler(dm_t* pDm);

/**
 * Pairs of xxx_handler() and xxx_thread_handler().
 * xx_handler() handles getting inputs from standard input, initialise, configuring all
 * required parameters for animation and creating new thread for running xxx_thread_handler().
 * xxx_thread_handler() is where a newly created thread execute infinitely until it is being
 * stopped.
 */
void scroll_handler(dm_t* pDm);
void* scroll_thread_handler(void* data);
void blink_handler(dm_t* pDm);
void* blink_thread_handler(void* data);
void invert_handler(dm_t* pDm);
void* invert_thread_handler(void* data);
void change_handler(dm_t* pDm);
void* change_thread_handler(void* data);


#if 0
/* Test function. To be deleted. */
void test(void) {
	uint32_t num = 3;
	dm_t dm;

	if (STATUS_OK != init(&dm, num)) {
		return;
	}

#if 0
	anim_scroll_t scroll;
	scroll.waitTimeMs = 200;
	scroll.stepSize = 1;
	anim_scroll_t scroll1;
	scroll1.waitTimeMs = 200;
	scroll1.stepSize = -1;

	/* scrolling example */
	anim_hScrollText(&scroll, dm.pDotMatrix, dm.pMax7219, "Chiong LAI");
	while(1) {
		anim_hScrollText(&scroll1, dm.pDotMatrix, dm.pMax7219, NULL);
		anim_hScrollText(&scroll, dm.pDotMatrix, dm.pMax7219, NULL);
	}
#endif

#if 0
	anim_blink_t blink;
	uint32_t on[] = {200, 200};
	uint32_t off[] = {500, 500};
	blink.len = 2;
	blink.onTimeMs = on;
	blink.offTimeMs = off;
	blink.waitTimeMs = 500;
	blink.layout = TEXT_LAYOUT_CENTER;

	/* blinking example */
	anim_blinkText(&blink, dm.pDotMatrix, dm.pMax7219, "HirH?");
	while(1) {
		anim_blinkText(&blink, dm.pDotMatrix, dm.pMax7219, NULL);
	}
#endif

#if 0
	anim_blink_t blink;
	uint32_t on[] = {200, 200};
	uint32_t off[] = {500, 500};
	blink.len = 2;
	blink.onTimeMs = on;
	blink.offTimeMs = off;
	blink.waitTimeMs = 500;
	blink.layout = TEXT_LAYOUT_CENTER;

	/* inverting example */
	anim_invertText(&blink, dm.pDotMatrix, dm.pMax7219, "HirH?");
	while(1) {
		anim_invertText(&blink, dm.pDotMatrix, dm.pMax7219, NULL);
	}
#endif

#if 0
	anim_change_t change;
	char *textArr[4] = {"Hell", "yeah", "I", "coll"};
	uint8_t isInvert[] = {1, 0, 0, 0};
	uint32_t timeMs[] = {1000, 500, 1000, 500};

	change.len = 3;
	change.layout = TEXT_LAYOUT_CENTER;
	change.isInvert = isInvert;
	change.timeMs = timeMs;

	/* changing example */
	anim_changeText(&change, dm.pDotMatrix, dm.pMax7219, textArr, 4);
	while(1) {
		anim_changeText(&change, dm.pDotMatrix, dm.pMax7219, textArr, 4);
	}
#endif

#if 1
	max7219_fill(dm.pMax7219, 0xFF);
	MAX7219_setIntensity(dm.pMax7219, 15);
#endif

}
#endif


int main (int argc, char** argv) {
	int option;
	dm_t dm;

//	test();

	signal(SIGINT, close_handler);
	signal(SIGQUIT, close_handler);
	signal(SIGABRT, close_handler);
	signal(SIGTERM, close_handler);

	option = 5;
	std_in_readInt(&option, "Please enter number of cascaded dot matrix units:\n> ");

	init(&dm, (uint32_t) option);

	while (1) {
		fprintf(stdout, "Please select types of animation:\n");
		fprintf(stdout, "\t1. Horizontal scrolling text\n");
		fprintf(stdout, "\t2. Blinking text\n");
		fprintf(stdout, "\t3. Inverting text\n");
		fprintf(stdout, "\t4. Changing text\n");
		fprintf(stdout, "\t5. Set brightness\n");

		option = 1;
		std_in_readInt(&option, "> ");

		switch(option) {
		case 2:
			blink_handler(&dm);
			break;
		case 3:
			invert_handler(&dm);
			break;
		case 4:
			change_handler(&dm);
			break;
		case 5:
			setIntensity_handler(&dm);
			break;
		default:
			/* case 1: scrolling text is default */
			scroll_handler(&dm);
			break;
		}
	}
	return 0;
}

int32_t init(dm_t* pDm, uint32_t numCascade) {
	max7219Cfg_t cfg;

	/* Setup hardware */
	if (1 != bcm2835_init()) {
		fprintf(stderr, "Failed to initialise bcm2835.");
		return STATUS_ERROR;
	}
	bcm2835_spi_begin();
	bcm2835_spi_setClockDivider(25); 	/* Result in 10MHz */
	bcm2835_spi_setChipSelectPolarity(MAX7219_CS_INDEX, LOW);
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);

	cfg.numCascade = numCascade;
	cfg.spiIdx = MAX7219_SPI_INDEX;
	cfg.cs = MAX7219_CS_INDEX;
	if (STATUS_OK != max7219_create(&pDm->pMax7219, &cfg)) {
		fprintf(stderr, "Failed to create max7219 instance.");
		return STATUS_ERROR;
	}

	MAX7219_testDisplay(pDm->pMax7219, MAX7219_VALUE_OFF);
	MAX7219_setIntensity(pDm->pMax7219, DOTMATRIX_INTENSITY);
	MAX7219_setDecodeMode(pDm->pMax7219, DOTMATRIX_DECODE_MODE);
	MAX7219_setScanLimit(pDm->pMax7219, DOTMATRIX_SCAN_LIMIT);
	MAX7219_clear(pDm->pMax7219);
	MAX7219_setDisplayEnabled(pDm->pMax7219, MAX7219_VALUE_ON);

	if (STATUS_OK != dotMatrix_create(&pDm->pDotMatrix, MAX_NUM_CHAR * DOTMATRIX_FONT_CHAR_WIDTH)) {
		fprintf(stderr, "Failed to create dot matrix instance.");
		return STATUS_ERROR;
	}

	return STATUS_OK;
}

void close_handler(int signum) {
	bcm2835_spi_end();
	bcm2835_close();

	fprintf(stdout, "Program exited...\n");
	exit(signum);	// terminate program
}

void setIntensity_handler(dm_t* pDm){
	int num = 0;

	std_in_readInt(&num, "Select brightness (0 - 15, inclusive):\n> ");
	MAX7219_setIntensity(pDm->pMax7219, num & 0xF);
}

void scroll_handler(dm_t* pDm) {
	int result;
	int num;
	anim_scroll_t animScroll;
	scroll_t scroll;
	pthread_t scrollThread;
	char tempStr[MAX_NUM_TEMP_CHAR];

	scroll.pDm = pDm;
	scroll.animScroll = &animScroll;
	scroll.isRunning = 1;
	scroll.str = (char*) malloc(MAX_NUM_CHAR);

	num = 1;
	std_in_readInt(&num, "Enter scroll step size (positive to scroll left, negative for right):\n> ");
	animScroll.stepSize = (uint32_t) num;

	num = 500;
	std_in_readInt(&num, "Enter wait time (ms) for each step:\n> ");
	animScroll.waitTimeMs = (uint32_t) num;

	do {
		result = std_in_readLine(scroll.str, MAX_NUM_CHAR, "Enter a text to scroll (max 256 chars):\n> ");
	} while (result == 0);

	animScroll.delayFxn = bcm2835_delay;

	pthread_create(&scrollThread, NULL, scroll_thread_handler, &scroll);

	do {
		result = std_in_readLine(tempStr, MAX_NUM_CHAR, "Enter 's' to stop:\n> ");
	} while (result == 0 || tempStr[0] != 's');

	scroll.isRunning = 0;
	fprintf(stdout, "Waiting for current scroll to complete...\n");
	pthread_join(scrollThread, NULL);

	free(scroll.str);
}

void* scroll_thread_handler(void* data) {
	scroll_t *scroll = (scroll_t*) data;

	anim_hScrollText(scroll->animScroll, scroll->pDm->pDotMatrix, scroll->pDm->pMax7219, scroll->str);
	while (scroll->isRunning) {
		anim_hScrollText(scroll->animScroll, scroll->pDm->pDotMatrix, scroll->pDm->pMax7219, NULL);
	}

	return NULL;
}

void blink_handler(dm_t* pDm) {
	int num;
	int result;
	uint32_t i;
	uint32_t maxChar;
	anim_blink_t animBlink;
	blink_t blink;
	pthread_t blinkThread;
	char tempStr[MAX_NUM_TEMP_CHAR];

	blink.pDm = pDm;
	blink.animBlink = &animBlink;
	blink.isRunning = 1;

	maxChar = (max7219_getNumCascade(pDm->pMax7219) * MAX7219_DIGIT_LENGTH) / DOTMATRIX_FONT_CHAR_WIDTH + 1;
	maxChar = maxChar < MAX_NUM_CHAR? maxChar : MAX_NUM_CHAR;
	blink.str = (char*) malloc(maxChar);

	num = 3;
	fprintf(stdout, "Enter text layout:\n");
	fprintf(stdout, "\t1. Left\n");
	fprintf(stdout, "\t2. Right\n");
	fprintf(stdout, "\t3. Center (default)\n");
	std_in_readInt(&num, "> ");
	animBlink.layout = (uint32_t) (num - 1);

	num = 1;
	std_in_readInt(&num, "Enter the number of blinks:\n> ");
	if (num > 0)
		animBlink.len = (uint32_t) num;

	animBlink.onTimeMs = (uint32_t*) malloc(sizeof(uint32_t) * animBlink.len);
	animBlink.offTimeMs = (uint32_t*) malloc(sizeof(uint32_t) * animBlink.len);
	for (i = 0; i < animBlink.len; i++) {
		animBlink.onTimeMs[i] = 500;
		fprintf(stdout, "[%d]: On time (ms) ", i+1);
		std_in_readInt(&num, "> ");
		if (num >= 0)
			animBlink.onTimeMs[i] = num;

		animBlink.offTimeMs[i] = 500;
		fprintf(stdout, "[%d]: Off time (ms) ", i+1);
		std_in_readInt(&num, "> ");
		if (num >= 0)
			animBlink.onTimeMs[i] = num;
	}

	animBlink.waitTimeMs = 500;
	std_in_readInt(&num, "Enter wait time (ms) before next blink cycle:\n> ");
	if (num >= 0)
		animBlink.waitTimeMs = (uint32_t) num;

	do {
		fprintf(stdout, "Enter a text to blink (max %d chars):\n", maxChar-1);
		result = std_in_readLine(blink.str, maxChar, "> ");
	} while (result == 0);

	animBlink.delayFxn = bcm2835_delay;

	pthread_create(&blinkThread, NULL, blink_thread_handler, &blink);

	do {
		result = std_in_readLine(tempStr, MAX_NUM_CHAR, "Enter 's' to stop:\n> ");
	} while (result == 0 || tempStr[0] != 's');

	blink.isRunning = 0;
	fprintf(stdout, "Waiting for current blink cycle to complete...\n");
	pthread_join(blinkThread, NULL);

	free(animBlink.onTimeMs);
	free(animBlink.offTimeMs);
	free(blink.str);
}

void* blink_thread_handler(void* data) {
	blink_t *blink = (blink_t*) data;

	anim_blinkText(blink->animBlink, blink->pDm->pDotMatrix, blink->pDm->pMax7219, blink->str);
	while (blink->isRunning) {
		anim_blinkText(blink->animBlink, blink->pDm->pDotMatrix, blink->pDm->pMax7219, NULL);
	}

	return NULL;
}

void invert_handler(dm_t* pDm) {
	int num;
	int result;
	uint32_t i;
	uint32_t maxChar;
	anim_invert_t animInvert;
	invert_t invert;
	pthread_t invertThread;
	char tempStr[MAX_NUM_TEMP_CHAR];

	invert.pDm = pDm;
	invert.animInvert = &animInvert;
	invert.isRunning = 1;

	maxChar = (max7219_getNumCascade(pDm->pMax7219) * MAX7219_DIGIT_LENGTH) / DOTMATRIX_FONT_CHAR_WIDTH + 1;
	maxChar = maxChar < MAX_NUM_CHAR? maxChar : MAX_NUM_CHAR;
	invert.str = (char*) malloc(maxChar);

	num = 3;
	fprintf(stdout, "Enter text layout:\n");
	fprintf(stdout, "\t1. Left\n");
	fprintf(stdout, "\t2. Right\n");
	fprintf(stdout, "\t3. Center (default)\n");
	std_in_readInt(&num, "> ");
	animInvert.layout = (uint32_t) (num - 1);

	num = 1;
	std_in_readInt(&num, "Enter the number of inverts:\n> ");
	if (num > 0)
		animInvert.len = (uint32_t) num;

	animInvert.onTimeMs = (uint32_t*) malloc(sizeof(uint32_t) * animInvert.len);
	animInvert.offTimeMs = (uint32_t*) malloc(sizeof(uint32_t) * animInvert.len);
	for (i = 0; i < animInvert.len; i++) {
		animInvert.onTimeMs[i] = 500;
		fprintf(stdout, "[%d] On time (ms) ", i+1);
		std_in_readInt(&num, "> ");
		if (num >= 0)
			animInvert.onTimeMs[i] = num;

		animInvert.offTimeMs[i] = 500;
		fprintf(stdout, "[%d] Off time (ms) ", i+1);
		std_in_readInt(&num, "> ");
		if (num >= 0)
			animInvert.onTimeMs[i] = num;
	}

	animInvert.waitTimeMs = 500;
	std_in_readInt(&num, "Enter wait time (ms) before next invert cycle:\n> ");
	if (num >= 0)
		animInvert.waitTimeMs = (uint32_t) num;

	do {
		fprintf(stdout, "Enter a text to invert (max %d chars):\n", maxChar-1);
		result = std_in_readLine(invert.str, maxChar, "> ");
	} while (result == 0);

	animInvert.delayFxn = bcm2835_delay;

	pthread_create(&invertThread, NULL, invert_thread_handler, &invert);

	do {
		result = std_in_readLine(tempStr, MAX_NUM_CHAR, "Enter 's' to stop:\n> ");
	} while (result == 0 || tempStr[0] != 's');

	invert.isRunning = 0;
	fprintf(stdout, "Waiting for current invert cycle to complete...\n");
	pthread_join(invertThread, NULL);

	free(animInvert.onTimeMs);
	free(animInvert.offTimeMs);
	free(invert.str);
}

void* invert_thread_handler(void* data) {
	invert_t *invert = (invert_t*) data;

	anim_invertText(invert->animInvert, invert->pDm->pDotMatrix, invert->pDm->pMax7219, invert->str);
	while (invert->isRunning) {
		anim_invertText(invert->animInvert, invert->pDm->pDotMatrix, invert->pDm->pMax7219, NULL);
	}

	return NULL;
}

void change_handler(dm_t* pDm) {
	int num;
	int result;
	uint32_t i;
	uint32_t maxChar;
	anim_change_t animChange;
	change_t change;
	pthread_t changeThread;
	char tempStr[MAX_NUM_TEMP_CHAR];

	change.pDm = pDm;
	change.animChange = &animChange;
	change.isRunning = 1;

	maxChar = (max7219_getNumCascade(pDm->pMax7219) * MAX7219_DIGIT_LENGTH) / DOTMATRIX_FONT_CHAR_WIDTH + 1;
	maxChar = maxChar < MAX_NUM_CHAR? maxChar : MAX_NUM_CHAR;

	num = 3;
	fprintf(stdout, "Enter text layout:\n");
	fprintf(stdout, "\t1. Left\n");
	fprintf(stdout, "\t2. Right\n");
	fprintf(stdout, "\t3. Center (default)\n");
	std_in_readInt(&num, "> ");
	animChange.layout = (uint32_t) (num - 1);

	num = 1;
	std_in_readInt(&num, "Enter the number of text to display:\n> ");
	if (num > 0) {
		animChange.len = (uint32_t) num;
		change.len = (uint32_t) num;
	}

	animChange.timeMs = (uint32_t*) malloc(sizeof(uint32_t) * animChange.len);
	animChange.isInvert = (uint8_t*) malloc(sizeof(uint8_t) * animChange.len);
	change.str = (char**) malloc(sizeof(char*) * animChange.len);

	for (i = 0; i < animChange.len; i++) {
		animChange.timeMs[i] = 500;
		fprintf(stdout, "[%d]: Time (ms):\n", i+1);
		std_in_readInt(&num, "> ");
		if (num >= 0)
			animChange.timeMs[i] = num;

		animChange.isInvert[i] = 0;
		fprintf(stdout, "[%d]: Invert (0 for normal, 1 for invert):\n", i+1);
		std_in_readInt(&num, "> ");
		if (num >= 0)
			animChange.isInvert[i] = num;

		change.str[i] = (char*) malloc(maxChar);
		do {
			fprintf(stdout, "[%d]: Enter text (max %d chars):\n", i+1, maxChar-1);
			result = std_in_readLine(change.str[i], maxChar, "> ");
		} while (result == 0);
	}

	animChange.delayFxn = bcm2835_delay;

	pthread_create(&changeThread, NULL, change_thread_handler, &change);

	do {
		result = std_in_readLine(tempStr, MAX_NUM_CHAR, "Enter 's' to stop:\n> ");
	} while (result == 0 || tempStr[0] != 's');

	change.isRunning = 0;
	fprintf(stdout, "Waiting for current change cycle to complete...\n");
	pthread_join(changeThread, NULL);

	free(animChange.timeMs);
	free(animChange.isInvert);
	for (i = 0; i < animChange.len; i++) {
		free(change.str[i]);
	}
	free(change.str);
}

void* change_thread_handler(void* data) {
	change_t *change = (change_t*) data;

	anim_changeText(change->animChange, change->pDm->pDotMatrix, change->pDm->pMax7219, (const char* const*) change->str, change->len);
	while (change->isRunning) {
		anim_changeText(change->animChange, change->pDm->pDotMatrix, change->pDm->pMax7219, (const char* const*) change->str, change->len);
	}

	return NULL;
}

