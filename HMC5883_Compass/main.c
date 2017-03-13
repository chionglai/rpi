
#include <stdio.h>
#include <stdint.h>
#include "util/status.h"
#include "module/hmc5883-compass/hmc5883.h"

#include "3p-lib/bcm2835-1.50/inc/bcm2835.h"

#define HCM5883_I2C_INDEX	(0)

//#define DRDY_PIN	-1 //RPI_V2_GPIO_P1_11

int main (void) {
//	int16_t x, y, z;
	float xf, yf, zf;
	hmc5883_t *hmc5883;
	hmc5883Cfg_t cfg;

	/* Setup hardware */
	if (!bcm2835_init())
		return 0;

	bcm2835_i2c_begin();
	// 400kHz clock speed. Fastest that can be supported by HMC5883 without
	// external pull-up resistor
	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_626);

	cfg.i2cIdx = HCM5883_I2C_INDEX;
	if (hmc5883_create(&hmc5883, &cfg) != STATUS_OK) {
		return -1;
	}
	
	hmc5883_setGain(hmc5883, HMC5883_GN_820);

	while (1) {
/*
		if (!hmc5883_read(&x, &y, &z)) {
			fprintf(stdout, "\rError reading compass!");
		} else if (x == HMC5883_VALUE_ERROR ) {
			fprintf(stdout, "\rX overflow/underflow!");
		} else if (y == HMC5883_VALUE_ERROR ) {
			fprintf(stdout, "\rY overflow/underflow!");
		} else if (z == HMC5883_VALUE_ERROR ) {
			fprintf(stdout, "\rZ overflow/underflow!");
		} else {
			fprintf(stdout, "\rX: %d  Y: %d  Z: %d     ", x, y, z);
		}
*/
		if (hmc5883_readGauss(hmc5883, &xf, &yf, &zf) != STATUS_OK) {
			fprintf(stdout, "\rError reading compass!");
		} else {
			fprintf(stdout, "\rX: %4.4f  Y: %4.4f  Z: %4.4f     ", xf, yf, zf);
		}

		fflush(stdout);
		bcm2835_delay(400);
	}
	
	hcm5883_destroy(&hmc5883);
	bcm2835_i2c_end();
	bcm2835_close();
	return 0;

}
