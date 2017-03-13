/*
 * main.c
 *
 *  Created on: 8 Mar 2016
 *      Author: lcc
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "util/util.h"
#include "util/bitmap.h"
#include "util/std.h"
#include "module/st7735-lcd/st7735.h"
#include "module/st7735-lcd/lcd.h"

#include "Fonts/FreeMonoBold12pt7b.h"
//#include "../Fonts/FreeMonoOblique12pt7b.h"

#include "3p-lib/bcm2835-1.50/inc/bcm2835.h"

/* Default pins. */
#define ST7735_SPI_INDEX		(0)
#define ST7735_CS_INDEX         BCM2835_SPI_CS0 /*< CS0 */
#define ST7735_PIN_DCX          RPI_GPIO_P1_18  /*< GPIO24 */

#define DELIMITER	" "
#define MAX_CHAR	256
#define MAX_INT		64


#if 0
/* test code */
void test (void) {
#if 0
    /* Test code */
    uint8_t i = 0;
    char str[8];
    lcd_setOrientation(pLcd, LCD_ORIENT_LANDSCAPE_INVERTED);
    lcd_setTextStroke(pLcd, LCD_COLOR_GREEN, LCD_COLOR_BLACK, 1, 2);
    LCD_fillScreen(pLcd, LCD_COLOR_RED);
    while (1) {
    	LCD_writeTextInCanvas(pLcd, 10, 5, -1, 50, LCD_LAYOUT_VER_BOTTOM | LCD_LAYOUT_HOR_RIGHT, "Hello");
    	snprintf(str, 8, "%d", i);
//    	LCD_writeTextInCanvasSpanX(pLcd, 100, 20, LCD_LAYOUT_HOR_CENTER | LCD_LAYOUT_VER_CENTER, str);
    	lcd_writeTextInCanvas(pLcd, 0, 100, -1, 0, LCD_LAYOUT_HOR_CENTER | LCD_LAYOUT_VER_CENTER, str, -1);
    	i++;
    	bcm2835_delay(1000);
    }
#endif

#if 0
    // draw random geometry
    lcd_fillRect(pLcd, 0, 0, pLcd->width, pLcd->height, 0xFF00FF);
    lcd_drawPixel(pLcd, 2, 2, 0xFF);
    lcd_drawPixel(pLcd, pLcd->width-3, 2, 0xFF);
    lcd_drawPixel(pLcd, 2, pLcd->height-3, 0xFF);
    lcd_drawPixel(pLcd, pLcd->width-3, pLcd->height-3, 0xFF);
    lcd_drawRect(pLcd, 40, 40, 20, 20, 0xFF00);
    lcd_drawCircle(pLcd, 100, 10, 20, 0xFFFF);
    //lcd_fillRect(pLcd, 0, pLcd->height/2, pLcd->width, pLcd->height, 0xFF00);
    lcd_drawFastVLine(pLcd, pLcd->width/2, 0, pLcd->height, 0xFFFF);
    lcd_drawLine(pLcd, 30, 30, 50, 50, 0xFF0000);
    lcd_drawLine(pLcd, 50, 50, 80, 30, 0xFF00);
    lcd_drawLine(pLcd, 80, 30, 50, 0, 0xFF);
    lcd_drawLine(pLcd, 50, 0, 30, 30, 0xFFFF);

    uint16_t x[10];
    uint16_t y[10];
    uint32_t color[40];
    uint32_t i;
    x[0] = 100; y[0] = 50;
	x[1] = 120; y[1] = 80;
	x[2] = 120; y[2] = 120;
	x[3] = 100; y[3] = 150;
	x[4] = 80; y[4] = 120;
	x[5] = 80; y[5] = 80;
    lcd_drawPolygon(pLcd, x, y, 6, 0xFFFF00);


    // Write text
    lcd_setTextStroke(pLcd, LCD_COLOR_YELLOW, LCD_COLOR_BLACK, 1);
    LCD_fillScreen(pLcd, pLcd->bgColor);
    LCD_write(pLcd, 0, 0, "Hello world!!");
    lcd_setFont(pLcd, &FreeMonoBold12pt7b);
//    lcd_setFont(pLcd, &FreeMonoOblique12pt7b);
    LCD_write(pLcd, 0, 0, "Hello world!!");
//    LCD_clear(pLcd, 0, 0, "Hello world!!");
    lcd_writeText(pLcd, 20, 70, "I am a genius!!", -1, LCD_COLOR_GREEN, LCD_COLOR_BLACK, 1);

    for (i = 0; i < 40; i++) {
    	color[i] = 0xFF;
    }
    lcd_drawBitmap(pLcd, 0, 0, 4, 10, color);
#endif
}

#endif

void printHelp(void) {
	printf("Command:\n");
	printf("os <orientation>:\n");
	printf("  Set screen orientation:\n");
	printf("  1: Portrait, 2: Portrait inverted, 3: Landscape, 4: Landscape inverted\n");
	printf("tp <x y str>:\n");
	printf("  Print string starting at (x, y).\n");
	printf("tc <color>:\n");
	printf("  24-bit RGB text color in hex.\n");
	printf("ts <size>:\n");
	printf("  Text size in positive int.\n");
	printf("bc <color>:\n");
	printf("  24-bit RGB background color in hex.\n");
	printf("dc <x y r color>:\n");
	printf("  Draw a circle with center at (x, y) and radius r.\n");
	printf("dr <x y w h color>:\n");
	printf("  Draw rectangle at (x, y) and width w and height h.\n");
	printf("dl <x0 y0 x1 y1 color>:\n");
	printf("  Draw a line from (x0, y0) to (x1, y1).\n");
	printf("di <x y filename>:");
	printf("  Draw image from *bmp file starting at (x, y). The bitmap file\n");
	printf("  must be saved as 24-bit bitmap format.\n");
	printf("fr <x y w h color>:\n");
	printf("  Fill rectangle. Args same as 'dr' command.\n");
	printf("fs <color>:\n");
	printf("  Fill/clear whole screen with color.\n");
	printf("h:\n");
	printf("  Print this help.\n");
	printf("q:\n");
	printf("  Quit.\n");
	printf("\n");
}


int main (int argc, char** argv) {
	char input[MAX_CHAR];
	char *p;
	uint32_t i32[MAX_INT];
	uint8_t i8;
	int32_t status;
    lcd_t *pLcd;
    st7735_t *pSt;
    bitmap_t *pBitmap;
    st7735Cfg_t cfg;

    /* setup hardware */
	if (1 != bcm2835_init()) {
		fprintf(stderr, "Failed to initialise bcm2835.");
		return STATUS_ERROR;
	}
	bcm2835_spi_begin();
	// Max SPI data speed for ST7735 is 15.1Mbit/s (write) and 6.7Mbit/s (read).
	// bcm2835 has base clock speed of 250MHz. Clock divider for bcm2835 must
	// be even, and 250/16 = 15.6MHz. I am using write speed since it is not
	// possible to read using this module.
	bcm2835_spi_setClockDivider(16);
	bcm2835_spi_setChipSelectPolarity(ST7735_CS_INDEX, LOW);
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
	// setup GPIO for DCX line
	bcm2835_gpio_fsel(ST7735_PIN_DCX, BCM2835_GPIO_FSEL_OUTP);

	cfg.model = ST7735_MODEL_B;
	cfg.spiIdx = ST7735_SPI_INDEX;
	cfg.csPin = ST7735_CS_INDEX;
	cfg.dcxPin = ST7735_PIN_DCX;
	cfg.delayMs = bcm2835_delay;
    st7735_create(&pSt,&cfg);
    st7735_panel_setColorFmt(pSt, ST7735_PANEL_COLOR_18_BIT);

    lcd_create(&pLcd, pSt);
    lcd_setOrientation(pLcd, LCD_ORIENT_PORTRAIT_NORMAL);

    printHelp();

    /* Example of setting to a custom font */
    //lcd_setFont(pLcd, &FreeMonoBold12pt7b);

//    test();


    while(std_in_readLine(input, MAX_CHAR, ">> ") == 0);
    while (input[0] != 'q') {
    	p = strtok(input, DELIMITER);
    	switch(p[0]) {
    	case 'h':
    		printHelp();
    		break;

    	case 'o':
    		switch(p[1]) {
    		case 's':
    			p = strtok(NULL, DELIMITER);
    			i32[0] = atoi(p);
    			switch(i32[0]) {
    			case 1:
    				lcd_setOrientation(pLcd, LCD_ORIENT_PORTRAIT_NORMAL);
    				break;
    			case 2:
    				lcd_setOrientation(pLcd, LCD_ORIENT_PORTRAIT_INVERTED);
    				break;
    			case 3:
    				lcd_setOrientation(pLcd, LCD_ORIENT_LANDSCAPE_NORMAL);
    				break;
    			case 4:
    				lcd_setOrientation(pLcd, LCD_ORIENT_LANDSCAPE_INVERTED);
    				break;
    			}
    			break;

    		default:
    			printf("ERROR: Unknown command '%s'!\n", p);
    			break;
    		}
    		break;

    	case 't':
    		switch(p[1]) {
    		case 'p':
				p = strtok(NULL, DELIMITER);
				sscanf(p, "%d", &i32[0]);
				p = strtok(NULL, DELIMITER);
				sscanf(p, "%d", &i32[1]);
				p = strtok(NULL, "\0");
				LCD_write(pLcd, i32[0], i32[1], p);
				break;

    		case 'c':
				p = strtok(NULL, DELIMITER);
				sscanf(p, "%x", &i32[0]);
				lcd_setTextStroke(pLcd, i32[0], pLcd->bgColor, pLcd->isBgOpaque, pLcd->textSize);
				break;

    		case 's':
				p = strtok(NULL, DELIMITER);
				sscanf(p, "%d", &i32[0]);
				lcd_setTextStroke(pLcd, pLcd->textColor, pLcd->bgColor, pLcd->isBgOpaque, i32[0]);
				break;

			default:
				printf("ERROR: Unknown command '%s'!\n", p);
				break;
			}
    		break;

    	case 'b':
    		switch(p[1]) {
    		case 'c':
    			p = strtok(NULL, DELIMITER);
    			sscanf(p, "%x", &i32[0]);
    			lcd_setTextStroke(pLcd, pLcd->textColor, i32[0], pLcd->isBgOpaque, pLcd->textSize);
    			break;

			default:
				printf("ERROR: Unknown command '%s'!\n", p);
				break;
			}
    		break;

    	case 'd':
    		switch(p[1]) {
    		case 'c':
    			p = strtok(NULL, DELIMITER);
				for (i8 = 0; i8 < 3 && p != NULL; i8++) {
					sscanf(p, "%d", &i32[i8]);
					p = strtok(NULL, DELIMITER);
				}
				sscanf(p, "%x", &i32[i8]);
				lcd_drawCircle(pLcd, i32[0], i32[1], i32[2], i32[3]);
    			break;

    		case 'r':
    			p = strtok(NULL, DELIMITER);
				for (i8 = 0; i8 < 4 && p != NULL; i8++) {
					sscanf(p, "%d", &i32[i8]);
					p = strtok(NULL, DELIMITER);
				}
				sscanf(p, "%x", &i32[i8]);
				lcd_drawRect(pLcd, i32[0], i32[1], i32[2], i32[3], i32[4]);
    			break;

    		case 'l':
    			p = strtok(NULL, DELIMITER);
				for (i8 = 0; i8 < 4 && p != NULL; i8++) {
					sscanf(p, "%d", &i32[i8]);
					p = strtok(NULL, DELIMITER);
				}
				sscanf(p, "%x", &i32[i8]);
				lcd_drawLine(pLcd, i32[0], i32[1], i32[2], i32[3], i32[4]);
    			break;

    		case 'i':
    			p = strtok(NULL, DELIMITER);
				sscanf(p, "%d", &i32[0]);
				p = strtok(NULL, DELIMITER);
				sscanf(p, "%d", &i32[1]);
				p = strtok(NULL, "\0");

				status = bitmap_readFile(&pBitmap, p);
				if (STATUS_OK == status) {
					i32[2] = pBitmap->imageHeader.width;
					i32[3] = pBitmap->imageHeader.height;
					lcd_drawBitmap(pLcd, i32[0], i32[1], i32[2], i32[3], pBitmap->image);
					bitmap_destroy(&pBitmap);
				}
				break;

			default:
				printf("ERROR: Unknown command '%s'!\n", p);
				break;
			}
    		break;

    	case 'f':
    		switch(p[1]) {
    		case 'r':
    			p = strtok(NULL, DELIMITER);
    			for (i8 = 0; i8 < 4 && p != NULL; i8++) {
    				sscanf(p, "%d", &i32[i8]);
    				p = strtok(NULL, DELIMITER);
    			}
    			sscanf(p, "%x", &i32[i8]);
    			lcd_fillRect(pLcd, i32[0], i32[1], i32[2], i32[3], i32[4]);
    			break;

    		case 's':
    			p = strtok(NULL, DELIMITER);
    			sscanf(p, "%x", &i32[0]);
    			LCD_fillScreen(pLcd, i32[0]);
    			break;

			default:
				printf("ERROR: Unknown command '%s'!\n", p);
				break;
			}
    		break;

    	default:
    		printf("ERROR: Unknown command '%s'!\n", p);
    		break;
    	}

    	while(std_in_readLine(input, MAX_CHAR, ">> ") == 0);
    }

    // power down ST7735 module
    st7735_destroy(&pSt);
    lcd_destroy(&pLcd);
	bcm2835_spi_end();
	bcm2835_close();

    return 0;
}

