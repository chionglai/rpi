

// The main function is mainly for testing the MPU6050 library

/**
 * I moved the *_updateReg(), *_readRegN() and *_writeRegN() from individual module into i2c.c, and now
 * reading directly a module's register value is not possible. And thus, this project no longer compiles
 * until that is fixed.
 */

#include <stdio.h>
#include "util/status.h"
#include "module/mpu6050-accelGyro/mpu6050.h"

#include "3p-lib/bcm2835-1.50/inc/bcm2835.h"

#define __TEST_AUX_I2C

#ifdef __TEST_AUX_I2C
#include "module/hmc5883-compass/hmc5883.h"
#endif

#define MPU6050_I2C_INDEX 	(0)
#define HCM5883_I2C_INDEX 	MPU6050_I2C_INDEX


// Passed black box test
void runSelfTest(mpu6050_t *mpu6050) {
	char result;

	// 1a. Print to tell user to keep the module still during self test
	fprintf(stdout, "Starting self test. Please ensure the MPU6050 module\n");
	fprintf(stdout, "is kept still while self-test is running.\n");
	fprintf(stdout, "Press any key to start.");
	getchar();

	result = mpu6050_accel_selfTest(mpu6050);
	if (result & MPU6050_ST_PASS_X) {
		fprintf(stdout, "Accelerometer X-axis PASSED.\n");
	} else {
		fprintf(stdout, "Accelerometer X-axis FAILED.\n");
	}
	if (result & MPU6050_ST_PASS_Y) {
		fprintf(stdout, "Accelerometer Y-axis PASSED.\n");
	} else {
		fprintf(stdout, "Accelerometer Y-axis FAILED.\n");
	}
	if (result & MPU6050_ST_PASS_Z) {
		fprintf(stdout, "Accelerometer Z-axis PASSED.\n");
	} else {
		fprintf(stdout, "Accelerometer Z-axis FAILED.\n");
	}
	result = mpu6050_gyro_selfTest(mpu6050);
	if (result & MPU6050_ST_PASS_X) {
		fprintf(stdout, "Gyroscope X-axis PASSED.\n");
	} else {
		fprintf(stdout, "Gyroscope X-axis FAILED.\n");
	}
	if (result & MPU6050_ST_PASS_Y) {
		fprintf(stdout, "Gyroscope Y-axis PASSED.\n");
	} else {
		fprintf(stdout, "Gyroscope Y-axis FAILED.\n");
	}
	if (result & MPU6050_ST_PASS_Z) {
		fprintf(stdout, "Gyroscope Z-axis PASSED.\n");
	} else {
		fprintf(stdout, "Gyroscope Z-axis FAILED.\n");
	}

}

// Passed black box test
void runBurstRead(mpu6050_t *mpu6050) {
	uint8_t buffer[24];
	int16_t value;
	int32_t sr;
	int32_t i, j;
	int32_t num;
	uint32_t delay;

	delay = 200;
	sr = 20;
	num = delay*sr/1000;

	fprintf(stdout, "Press any key to start MPU6050 module for burst read.");
	getchar();

	i = mpu6050_setSampleRate(mpu6050, sr);
	i = mpu6050_fifo_disable(mpu6050, MPU6050_FIFO_ALL);	 // disable all FIFO
	i = mpu6050_fifo_reset(mpu6050);
	i = mpu6050_fifo_enable(mpu6050, MPU6050_FIFO_ACCEL_XYZ);	// enable only the one we want

	while(1) {
		bcm2835_delay(delay);

		i = mpu6050_fifo_read(mpu6050, buffer, 24);
		fprintf(stdout, "Accelerometer: %d bytes read\n", i);
		fprintf(stdout, "       X         Y         Z\n");

		for (i = 0; i < num; i++) {
			for (j = 0; j < 3; j++) {
				value = uint8ToUint16(buffer, 6*i+2*j);
				fprintf(stdout, " %+9d", value);
			}
			fprintf(stdout, "\n");
		}

		printf("\033[A\033[A\033[A\033[A\033[A\033[A\r");
		fflush(stdout);
	}
}

// Passed black box test
void runNormal(mpu6050_t *mpu6050) {
	int16_t tempi, xi, yi, zi;
	float tempd, xd, yd, zd;

	fprintf(stdout, "Press any key to start MPU6050 module for normal read.");
	getchar();

	while(1) {
		if(mpu6050_temp_readRaw(mpu6050, &tempi) == STATUS_OK) {
			fprintf(stdout, "Temperature:\n\tRaw     = %+d", tempi);
		} else {
			fprintf(stdout, "Temperature:\n\tRaw     = ERROR");
		}

		if(mpu6050_temp_read(mpu6050, &tempd) == STATUS_OK) {
			fprintf(stdout, "\n\tCelcius = %+f", tempd);
		} else {
			fprintf(stdout, "\n\tCelcius = ERROR");
		}

		if (mpu6050_accel_readRaw(mpu6050, &xi, &yi, &zi) == STATUS_OK) {
			fprintf(stdout, "\nAccelerometer:\n\tRaw = %+7d | %+7d | %+7d", xi, yi, zi);
		} else {
			fprintf(stdout, "\nAccelerometer:\n\tRaw = ERROR");
		}

		if (mpu6050_accel_read(mpu6050, &xd, &yd, &zd) == STATUS_OK) {
			fprintf(stdout, "\n\tG's = %+3.4f | %+3.4f | %+3.4f", xd, yd, zd);
		} else {
			fprintf(stdout, "\n\tG's = ERROR");
		}

		if (mpu6050_gyro_readRaw(mpu6050, &xi, &yi, &zi) == STATUS_OK) {
			fprintf(stdout, "\nGyroscope:\n\tRaw   = %+7d | %+7d | %+7d", xi, yi, zi);
		} else {
			fprintf(stdout, "\nGyroscope:\n\tRaw   = ERROR");
		}

		if (mpu6050_gyro_read(mpu6050, &xd, &yd, &zd) == STATUS_OK) {
			fprintf(stdout, "\n\tDeg/s = %+3.4f | %+3.4f | %+3.4f", xd, yd, zd);
		} else {
			fprintf(stdout, "\n\tDeg/s = ERROR");
		}

		// move the cursor up 8 lines and rewind to first column
		printf("\033[A\033[A\033[A\033[A\033[A\033[A\033[A\033[A\r");
		fflush(stdout);

		bcm2835_delay(100);
	}
}

// Passed black box test.
void runCycle(mpu6050_t *mpu6050) {
	mpu6050_setMode(mpu6050, MPU6050_CYCLE, MPU6050_WAKEUP_FREQ_1_25);

	runNormal(mpu6050);
}

// Only motion detect pass
// still need to test for free fall and zero motion
void runMotionDetect(mpu6050_t *mpu6050) {
	uint8_t status;
	uint8_t intStatus;
	int16_t xi, yi, zi;
	float xd, yd, zd;

	mpu6050_setSampleRate(mpu6050, 4);
//	mpu6050_accel_setHpf(mpu6050, MPU6050_HPF_5);

	// todo: the threshold and duration values have not been tuned!!
	mpu6050_setMotion(mpu6050, 10, 1, MPU6050_COUNT_DEC4);
	mpu6050_setZeroMotion(mpu6050, 200, 1);
	mpu6050_setFreeFall(mpu6050, 10, 1, MPU6050_COUNT_DEC4);

	// need to enable interrupt
	mpu6050_updateReg(mpu6050,
			MPU6050_REG_INT_ENABLE,
			MPU6050_INT_FF|MPU6050_INT_MOT|MPU6050_INT_ZMOT,
			MPU6050_INT_FF|MPU6050_INT_MOT|MPU6050_INT_ZMOT);

//	bcm2835_delay(100);
//	mpu6050_accel_setHPF(mpu6050, MPU6050_HPF_5);
//	mpu6050_accel_disable(mpu6050, MPU6050_AG_X|MPU6050_AG_Z);

	while(1) {
		bcm2835_delay(200);
		mpu6050_readRegN(mpu6050, MPU6050_REG_MOT_DETECT_STATUS, &status, 1);
		mpu6050_readRegN(mpu6050, MPU6050_REG_INT_STATUS, &intStatus, 1);

		fprintf(stdout, "Interrupts:\n");
		if (intStatus & MPU6050_INT_FF) {
			fprintf(stdout, "\tFree fall: YES\n");
		} else {
			fprintf(stdout, "\tFree fall: NO \n");
		}

		if (intStatus & MPU6050_INT_MOT) {
			fprintf(stdout, "\tMotion: YES\n");
		} else {
			fprintf(stdout, "\tMotion: NO \n");
		}

		if (intStatus & MPU6050_INT_ZMOT) {
			fprintf(stdout, "\tZero Motion: YES\n");
		} else {
			fprintf(stdout, "\tZero Motion: NO \n");
		}

		if (status & MPU6050_ZRMOT) {
			fprintf(stdout, "Motion STOP detected. \n");
		} else {
			fprintf(stdout, "Motion START detected.\n");
		}

		if (status & MPU6050_MOT_XNEG) {
			fprintf(stdout, "\tMotion detected on --X. \n");
		} else if (status & MPU6050_MOT_XPOS) {
			fprintf(stdout, "\tMotion detected on ++X. \n");
		} else {
			fprintf(stdout, "\tNO Motion detected on X.\n");
		}

		if (status & MPU6050_MOT_YNEG) {
			fprintf(stdout, "\tMotion detected on --Y. \n");
		} else if (status & MPU6050_MOT_YPOS) {
			fprintf(stdout, "\tMotion detected on ++Y. \n");
		} else {
			fprintf(stdout, "\tNO motion detected on Y.\n");
		}

		if (status & MPU6050_MOT_ZNEG) {
			fprintf(stdout, "\tMotion detected on --Z. \n");
		} else if (status & MPU6050_MOT_ZPOS) {
			fprintf(stdout, "\tMotion detected on ++Z. \n");
		} else {
			fprintf(stdout, "\tNO motion detected on Z.\n");
		}

		if (mpu6050_accel_readRaw(mpu6050, &xi, &yi, &zi)) {
			fprintf(stdout, "\nAccelerometer:\n\tRaw = %+7d | %+7d | %+7d", xi, yi, zi);
		}

		if (mpu6050_accel_read(mpu6050, &xd, &yd, &zd)) {
			fprintf(stdout, "\n\tG's = %+3.4f | %+3.4f | %+3.4f", xd, yd, zd);
		}

		fprintf(stdout, "\nValue = 0x%02x\t0x%02x", status, intStatus);
		fprintf(stdout, "\033[A\033[A\033[A\033[A\033[A\033[A\033[A\033[A\033[A\r");
		fflush(stdout);
	}
}

#ifdef __TEST_AUX_I2C
// pass
void runI2cBypass(mpu6050_t *mpu6050) {
	float xf, yf, zf;
	hmc5883_t *hmc5883;
	hmc5883Cfg_t cfg;

	// set aux i2c on mpu6050 to bypass mode
	mpu6050_i2c_setMode(mpu6050, MPU6050_I2C_BYPASS);

	// create hmc5883 compass module
	cfg.i2cIdx = HCM5883_I2C_INDEX;
	hmc5883_create(&hmc5883, &cfg);

	while (1) {
		if (hmc5883_readGauss(hmc5883, &xf, &yf, &zf) != STATUS_OK) {
			fprintf(stdout, "Error reading compass!");
		} else {
			fprintf(stdout, "X: %4.4f  Y: %4.4f  Z: %4.4f     ", xf, yf, zf);
		}
		printf("\r");
		fflush(stdout);
		bcm2835_delay(200);
	}
}

// pass, with occasional i2c read error
// reason for error may be because there is only one i2c controller in mpu6050.
// while the i2c controller is communicating thru aux i2c with external sensor,
// it may not respond to read/write request on the primary i2c bus.
void runAuxI2c(mpu6050_t *mpu6050) {
	float tempd, xd, yd, zd;
	int16_t temp;
	uint8_t buffer[24];
	int32_t i;
	mpu6050I2cSlave slave;
	hmc5883_t *hmc5883;
	hmc5883Cfg_t cfg;

	// set aux i2c on mpu6050 to bypass mode
	mpu6050_i2c_setMode(mpu6050, MPU6050_I2C_BYPASS);

	// create hmc5883 compass module
	cfg.i2cIdx = HCM5883_I2C_INDEX;
	hmc5883_create(&hmc5883, &cfg);

	slave = 0;
	// setup hmc5883 as i2c slave 0 on mpu6050
	mpu6050_i2c_initSlave(mpu6050, slave, HMC5883_I2C_ADDR, HMC5883_REG_DOX_MSB);

	// set read parameter
	mpu6050_i2c_setReadFromSlave(mpu6050, slave, 6);

	mpu6050_updateReg(mpu6050,
			MPU6050_REG_I2C_SLV_CTRL(slave),
			MPU6050_I2C_SLV_REG_DIS|MPU6050_I2C_SLV_GRP,	// disable sending register addr before every byte read
			MPU6050_I2C_SLV_GRP);	// enable GRP since DOX_MSB is odd, need to group to form a word
	mpu6050_i2c_disableSlave(mpu6050, MPU6050_I2C_SLV_ALL);
	mpu6050_i2c_enableSlave(mpu6050, slave);

#if 0
	// repeat hmc5883 on another slave
	slave = 2;
	mpu6050_i2c_initSlave(mpu6050, slave, HMC5883_I2C_ADDR, HMC5883_REG_DOX_MSB);
	mpu6050_i2c_setReadFromSlave(mpu6050, slave, 6);
	mpu6050_updateReg(mpu6050,
			MPU6050_REG_I2C_SLV_CTRL(slave),
			MPU6050_I2C_SLV_REG_DIS|MPU6050_I2C_SLV_GRP,	// disable sending register addr before every byte read
			MPU6050_I2C_SLV_GRP);	// enable GRP since DOX_MSB is odd, need to group to form a word
	mpu6050_i2c_enableSlave(mpu6050, slave);
#endif

//	mpu6050_updateReg(mpu6050, 0x24, 0x10, 0x10);
//	mpu6050_updateReg(mpu6050, MPU6050_REG_PWR_MGMT1, MPU6050_MASK_CLK_SEL, MPU6050_CLKSEL_INT_8MHZ);
//	mpu6050_updateReg(mpu6050, MPU6050_REG_I2C_MST_CTRL, MPU6050_MASK_I2C_MST_CLK, MPU6050_I2C_MST_CLK_400);

	// enable i2c master on mpu6050
	mpu6050_i2c_setMode(mpu6050, MPU6050_I2C_NORMAL);

	while(1) {
		do {
			mpu6050_readRegN(mpu6050, MPU6050_REG_INT_STATUS, buffer, 1);
		} while ((buffer[0] & MPU6050_INT_DATA_RDY) == 0);

		if(mpu6050_temp_read(mpu6050, &tempd) == STATUS_OK) {
			fprintf(stdout, "\n\tCelcius = %+f", tempd);
		} else {
			fprintf(stdout, "\n\tCelcius = ERROR");
		}

		if (mpu6050_accel_read(mpu6050, &xd, &yd, &zd) == STATUS_OK) {
			fprintf(stdout, "\n\tG's = %+3.4f | %+3.4f | %+3.4f", xd, yd, zd);
		} else {
			fprintf(stdout, "\n\tG's = ERROR");
		}

		if (mpu6050_gyro_read(mpu6050, &xd, &yd, &zd) == STATUS_OK) {
			fprintf(stdout, "\n\tDeg/s = %+3.4f | %+3.4f | %+3.4f", xd, yd, zd);
		} else {
			fprintf(stdout, "\n\tDeg/s = ERROR");
		}

		if (mpu6050_readRegN(mpu6050, MPU6050_REG_EXT_SENS_DATA(0), buffer, 24) == STATUS_OK) {
			fprintf(stdout, "\nCompass raw:           ");
			for (i = 0; i < 24; i++) {
				if ((i % 6) == 0)
					fprintf(stdout, "\n\t");

				fprintf(stdout, "0x%02x   ", buffer[i]);
			}

			temp = uint8ToUint16(buffer, 0);
			xd = temp/1090.0;

			temp = uint8ToUint16(buffer, 2);
			zd = temp/1090.0;

			temp = uint8ToUint16(buffer, 4);
			yd = temp/1090.0;

			fprintf(stdout, "\n\tCompass = %+3.4f | %+3.4f | %+3.4f", xd, yd, zd);
		} else {
			// repeating printf in case of error reading hmc5883 from mpu6050 so that
			// the line clear still match.
			fprintf(stdout, "\n\tCompass = ERROR");
			fprintf(stdout, "\n\tCompass = ERROR");
			fprintf(stdout, "\n\tCompass = ERROR");
			fprintf(stdout, "\n\tCompass = ERROR");
			fprintf(stdout, "\n\tCompass = ERROR");
			fprintf(stdout, "\n\tCompass = ERROR");
		}

		mpu6050_readRegN(mpu6050, MPU6050_REG_I2C_MST_STATUS, buffer, 1);
		fprintf(stdout, "\n\tStatus = 0x%02x\n", buffer[0]);
		printf("\033[A\033[A\033[A\033[A\033[A\033[A\033[A\033[A\033[A\033[A\033[A\r");
		fflush(stdout);

		bcm2835_delay(100);
	}
}
#endif

void printHelp(void) {
	printf("MPU6050_AccelGyro options\n\n");
	printf("option:\n");
	printf("    1: Normal run.\n");
	printf("    2: Motion detect.\n");
	printf("    3: Burst read.\n");
	printf("    4: Cycle.\n");
	printf("    5: Self test.\n");
	printf("    6: Calibrate.\n");
#ifdef __TEST_AUX_I2C
	printf("    7: I2C bypass with HCM5883 sensor.\n");
	printf("    8: Aux I2C with HCM5883 sensor.\n");
#endif
}

int main(int argc, char *argv[]) {
	mpu6050Cfg_t cfg;
	mpu6050_t *mpu6050;

	/* setup hardware */
	if (!bcm2835_init())
		return 0;

	bcm2835_i2c_begin();
	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_626);

	cfg.i2cIdx = MPU6050_I2C_INDEX;
	if (mpu6050_create(&mpu6050, &cfg) != STATUS_OK) {
		fprintf(stderr, "ERROR: Failed to create MPU6050!\n");
		return 1;
	}

	if (argc < 2) {
		printHelp();
		goto exit;
	}

	/* Enable all sensors */
	mpu6050_accel_enable(mpu6050, MPU6050_AG_XYZ);
	mpu6050_temp_enable(mpu6050);
	mpu6050_gyro_enable(mpu6050, MPU6050_AG_XYZ);
	switch (argv[1][0]) {
	case '1':
		runNormal(mpu6050);
		break;
	case '2':
		runMotionDetect(mpu6050);
		break;
	case '3':
		runBurstRead(mpu6050);
		break;
	case '4':
		runCycle(mpu6050);
		break;
	case '5':
		runSelfTest(mpu6050);
		break;
	case '6':
		mpu6050_calibrate(mpu6050);
		break;
#ifdef __TEST_AUX_I2C
	case '7':
		runI2cBypass(mpu6050);
		break;
	case '8':
		runAuxI2c(mpu6050);
		break;
#endif
	default:
		printHelp();
		break;
	}

exit:
	/* Clean up */
	mpu6050_destroy(&mpu6050);
	bcm2835_i2c_end();
	bcm2835_close();

	return 0;
}
