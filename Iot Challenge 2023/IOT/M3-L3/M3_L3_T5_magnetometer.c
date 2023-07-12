/*************************************************
 *         © Keysight Technologies 2019 
 *
 * PROPRIETARY RIGHTS of Keysight Technologies are 
 * involved in the subject matter of this software. 
 * All manufacturing, reproduction, use, and sales 
 * rights pertaining to this software are governed 
 * by the license agreement. The recipient of this 
 * code implicitly accepts the terms of the license. 
 *
 ***************************************************
 *
 * FILE NAME  :  M3-L3-Magnetometer.c
 * DESCRIPTION:  This program reads Magnetometer values and write to an output file.
 *
 **************************************************/

#include "mraa_beaglebone_pinmap.h"
#include <time.h>
#include <math.h>
#include <stdbool.h>

#define LCD_ADDR 		 0x3E
// check the clave address : as of now keeping SA0 =1
#define I2C_SLAVE_ADDR_MAG 	0x1E

#define CTRL_REG1_M		0x20
#define CTRL_REG2_M		0x21
#define CTRL_REG3_M		0x22
#define CTRL_REG4_M		0x23
#define CTRL_REG5_M		0x24

#define FIFO_CTRL			0x2E
#define FIFO_SRC			0x2F
#define OUT_X_L_M	0x28

#define settings_MAG_enabled 	true
#define	settings_MAG_scale	4
#define	settings_MAG_sampleRate 	7
#define	settings_MAG_tempCompensationEnable	false
#define	settings_MAG_XYPerformance 3
#define	settings_MAG_ZPerformance 3
#define	settings_MAG_lowPowerEnable false
#define	settings_MAG_operatingMode 0

#define SENSITIVITY_MAGNETIC_2	0.00014
#define SENSITIVITY_MAGNETIC_4	0.00029
#define SENSITIVITY_MAGNETIC_8	0.00043
#define SENSITIVITY_MAGNETIC_16	0.00058

#define DELTA_T  0.01
#define COEFF    0.98
#define RAD_TO_DEG 180 / M_PI

#define BILLION 1E9

mraa_i2c_context I2Chandle_mag;
mraa_uart_context UARThandle;

float mRes;
float MagVal[3];
int16_t mx, my, mz; // x, y, and z axis readings of the magnetometer

struct timespec start, stop;
float time_elapse;
FILE *fp;

void Magnetometer_Init()
{

	mRes = SENSITIVITY_MAGNETIC_16;

    uint8_t tempRegValue = 0;
    
	if(settings_MAG_tempCompensationEnable)	tempRegValue |= (1<<7);	
	tempRegValue |= ((settings_MAG_XYPerformance & 0x3)<<5);
	tempRegValue |= ((settings_MAG_sampleRate & 0x7)<<2);
	
	mraa_i2c_write_byte_data(I2Chandle_mag,tempRegValue,CTRL_REG1_M);
		
	tempRegValue = 0;
	switch(settings_MAG_scale) {
		case 8:
			tempRegValue |= (0x1 << 5);
			break;
		case 12:
			tempRegValue |= (0x2 << 5);
			break;
		case 16:
			tempRegValue |= (0x3 << 5);
			break;
	}

	mraa_i2c_write_byte_data(I2Chandle_mag,tempRegValue,CTRL_REG2_M);

	tempRegValue = 0;

	if(settings_MAG_lowPowerEnable) tempRegValue |= (1<<5);
	tempRegValue |= (settings_MAG_operatingMode & 0x3);

	mraa_i2c_write_byte_data(I2Chandle_mag,tempRegValue,CTRL_REG3_M);


	tempRegValue = 0;
	tempRegValue = (settings_MAG_ZPerformance & 0x3) << 2;
	
	mraa_i2c_write_byte_data(I2Chandle_mag,tempRegValue,CTRL_REG4_M);

	tempRegValue = 0;
	mraa_i2c_write_byte_data(I2Chandle_mag,tempRegValue,CTRL_REG5_M);
	
}

void ReadMagneto() {
	uint8_t tempBuffer[6];
	mraa_i2c_read_bytes_data(I2Chandle_mag, OUT_X_L_M, tempBuffer, 6);
	mx = (tempBuffer[1] << 8) | tempBuffer[0];
	my = (tempBuffer[3] << 8) | tempBuffer[2];
	mz = (tempBuffer[5] << 8) | tempBuffer[4];

	MagVal[0] = mRes * mx;
	MagVal[1] = mRes * my;
	MagVal[2] = mRes * mz;
}

void ReadSensorValues()
{
    fp = fopen("magResults.csv","w");

	char buf[20];
	int ret;
	while(1){
		ReadMagneto();
        fprintf(fp, "%5.5f, %5.5f, %5.5f\n", MagVal[0], MagVal[1], MagVal[2]);
		printf("Mag:,%5.5f, %5.5f, %5.5f\n", MagVal[0], MagVal[1], MagVal[2]);
	}

    // int i;
    // clock_gettime (CLOCK_REALTIME, &start);
    // for(i=0; i < 2000; i++) {
    //     ReadMagneto();
    //     fprintf(fp, "%5.5f, %5.5f, %5.5f\n", MagVal[0], MagVal[1], MagVal[2]);
    //     printf("%5.5f, %5.5f, %5.5f\n", MagVal[0], MagVal[1], MagVal[2]);
    //     usleep(2250);
    // }
    // clock_gettime (CLOCK_REALTIME, &stop);
    // time_elapse = (stop.tv_sec + (float) stop.tv_nsec / BILLION) - (start.tv_sec + (float)start.tv_nsec / BILLION);
    // printf("Total time elapse: %f seconds\n", time_elapse);
    // // fprintf(fp, "Total time elapse: %f seconds\n", time_elapse);

    fclose(fp);

}

int main(void) {
  	int status; 
	char dev_string[] = "/dev/ttyS1";	

	I2Chandle_mag = mraa_i2c_init_raw (I2CS_BUS); // write correct I2C dev number here
	if (!I2Chandle_mag) { // The bus is not there 
		printf("I2C Bus 1 is not available. Please check /dev/i2c-2 \n");
		printf(" -> Exiting program\n");
		mraa_i2c_stop(I2Chandle_mag);
		mraa_deinit();
		return EXIT_FAILURE;
	}

   	mraa_i2c_frequency (I2Chandle_mag, MRAA_I2C_STD);
	mraa_i2c_address(I2Chandle_mag, I2C_SLAVE_ADDR_MAG);
	status = mraa_i2c_read_byte(I2Chandle_mag);
	if (status < 0){
        printf("Failed to Initialize Magnetometer -> Exiting program\n");
		printf("Check i2cdetect -r -y 1 for 0x1E\n");
		mraa_i2c_stop(I2Chandle_mag);
		mraa_deinit();
		return EXIT_FAILURE;
	}

	// UART
	UARThandle = mraa_uart_init_raw(dev_string);
        if(UARThandle == NULL) {
		printf("Failed to setup UART\n");
		return EXIT_FAILURE;
	}
	mraa_uart_set_baudrate(UARThandle, 9600);
	mraa_uart_set_mode(UARThandle, 8, MRAA_UART_PARITY_NONE, 1);
	mraa_uart_set_flowcontrol(UARThandle, 0, 0);
	mraa_uart_set_timeout(UARThandle, 0, 0, 0);

	Magnetometer_Init();
	ReadSensorValues();
	
	return 0;
}

