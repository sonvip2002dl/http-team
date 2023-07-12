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
 * FILE NAME  :  M3_L3_T4_tiltedAngle.c
 * DESCRIPTION:  This program reads Gyroscope values and write the output 
 *				 (time elapse for each output, and the values of the axis)
 * 				 into a CSV file.
 *
 **************************************************/

#include "mraa_beaglebone_pinmap.h"
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define LCD_ADDR 		 0x3E
// check the clave address : as of now keeping SA0 =1
#define I2C_SLAVE_ADDR 		0x6B
#define CTRL_REG1_G			0x10
#define CTRL_REG2_G			0x11
#define CTRL_REG3_G			0x12
#define CTRL_REG4           0x1E
#define CTRL_REG9           0x23
#define FIFO_CTRL			0x2E
#define FIFO_SRC			0x2F
#define OUT_X_L_G           0x18
#define SENSITIVITY_GYROSCOPE_245    0.00875
#define SENSITIVITY_GYROSCOPE_500    0.0175
#define SENSITIVITY_GYROSCOPE_2000 	 0.07
#define DELTA_T  0.01
#define COEFF    0.98
#define RAD_TO_DEG 180 / M_PI
#define BILLION 1E9

mraa_i2c_context I2Chandle;
mraa_uart_context UARThandle;

float gRes;
float GyroVal[3];
int16_t gx, gy, gz; // x, y, and z axis readings of the gyroscope
struct timespec start, stop, instant;
double time1, time2, time_elapse;
FILE *fp;

void Gyroscope_Init()
{
	// Gyroscope settings
	uint8_t tempRegValue = 0;
	uint8_t sampleRate;
	uint8_t bandwidth;
	uint8_t scale;

	// Initialize CTRL_REG1_G (10h) register :[ODR_G2][ODR_G1][ODR_G0][FS_G1][FS_G0][0][BW_G1][BW_G0]

	// FS_G- gyro sample rate: value between 1-6
	// 1 = 14.9    4 = 238
	// 2 = 59.5    5 = 476
	// 3 = 119     6 = 952
	sampleRate = 6;

	// ODR_G- gyro scale can be 245, 500, or 2000
	scale = 245; // Setting default to 245
	gRes = SENSITIVITY_GYROSCOPE_245;

	// gyro cutoff frequency: value between 0-3
	// Actual value of cutoff frequency depends on sample rate.
	bandwidth = 0;

	tempRegValue = (sampleRate & 0x07) << 5;

	if (scale == 500)
		tempRegValue |= (0x1 << 3);

	else if(scale ==2000)	
		tempRegValue |= (0x3 << 3);

	// else we'll set it to 245 dps (0x0 << 4)

	//BW_G -
	tempRegValue |= (bandwidth & 0x3);

	mraa_i2c_write_byte_data(I2Chandle,tempRegValue,CTRL_REG1_G);

	// Initialize CTRL_REG2_G (Default value: 0x00): [0][0][0][0][INT_SEL1][INT_SEL0][OUT_SEL1][OUT_SEL0]
	mraa_i2c_write_byte_data(I2Chandle,0x00,CTRL_REG2_G);	

	// CTRL_REG3_G (Default value: 0x00): [LP_mode][HP_EN][0][0][HPCF3_G][HPCF2_G][HPCF1_G][HPCF0_G]
	mraa_i2c_write_byte_data(I2Chandle, 0x00, CTRL_REG3_G);

	// CTRL_REG4 (Default value: 0x38): [0][0][Zen_G][Yen_G][Xen_G][0][LIR_XL1][4D_XL1]
	tempRegValue = 0;
	tempRegValue |= (1<<5);
	tempRegValue |= (1<<4);
	tempRegValue |= (1<<3);
	//tempRegValue |= (1<<1); //Latched Interrupt ?? need to check the behavior
	mraa_i2c_write_byte_data(I2Chandle,tempRegValue,CTRL_REG4);

	// ORIENT_CFG_G (Default value: 0x00):[0][0][SignX_G][SignY_G][SignZ_G][Orient_2][Orient_1][Orient_0]

}

void ReadGyro() {
	uint8_t tempBuffer[6];

	mraa_i2c_read_bytes_data(I2Chandle, OUT_X_L_G, tempBuffer, 6);

	gx = (tempBuffer[1] << 8) | tempBuffer[0]; // Store x-axis values into gx
	gy = (tempBuffer[3] << 8) | tempBuffer[2]; // Store y-axis values into gy
	gz = (tempBuffer[5] << 8) | tempBuffer[4]; // Store z-axis values into gz

	GyroVal[0] = gRes * gx;
	GyroVal[1] = gRes * gy;
	GyroVal[2] = gRes * gz;
}

void ReadSensorValues()
{
	fp = fopen("tiltedresults.csv","w");

	char buf[20];
	int ret;
    int i;
	
    clock_gettime (CLOCK_REALTIME, &start);
    time1 = start.tv_sec + (float) start.tv_nsec / BILLION;
    // while(1){
	// 	ReadGyro();
	// 	clock_gettime(CLOCK_REALTIME, &instant);
    //     time2 = instant.tv_sec + (float) instant.tv_nsec / BILLION;
    //     fprintf(fp, "%5.5f, %5.5f, %5.5f, %5.5f\n", time2-time1, GyroVal[0], GyroVal[1], GyroVal[2]);
    //     printf("%5.5f, %5.5f, %5.5f, %5.5f\n", time2-time1, GyroVal[0], GyroVal[1], GyroVal[2]);
	// 	time1 = time2;
	// }

	for(i=0; i < 2000; i++) {
        ReadGyro();
        clock_gettime(CLOCK_REALTIME, &instant);
        time2 = instant.tv_sec + (float) instant.tv_nsec / BILLION;
        fprintf(fp, "%5.5f, %5.5f, %5.5f, %5.5f\n", time2-time1, GyroVal[0], GyroVal[1], GyroVal[2]);
        printf("%5.5f, %5.5f, %5.5f, %5.5f\n", time2-time1, GyroVal[0], GyroVal[1], GyroVal[2]);
        time1 = time2;
        usleep(1000);
    }
    clock_gettime (CLOCK_REALTIME, &stop);
    
    time_elapse = stop.tv_sec + (float) stop.tv_nsec / BILLION - (start.tv_sec + (float)start.tv_nsec / BILLION);
    printf("Total time elapse: %f seconds\n", time_elapse);
    // fprintf(fp, "Total time elapse: %f seconds\n", time_elapse);

    
    fclose(fp);

}

int main(void) {
  	int status; 
	char dev_string[] = "/dev/ttyS1";	
	// I2C
	I2Chandle = mraa_i2c_init_raw (I2CS_BUS); // write correct I2C dev number here
	if (!I2Chandle) { // The bus is not there 
		printf("I2C Bus 1 is not available. Please check /dev/i2c-2 \n");
		printf(" -> Exiting program\n");
		mraa_i2c_stop(I2Chandle);
		mraa_deinit();
		return EXIT_FAILURE;
	}

   	mraa_i2c_frequency (I2Chandle, MRAA_I2C_STD);
	mraa_i2c_address(I2Chandle, I2C_SLAVE_ADDR);
	status = mraa_i2c_read_byte(I2Chandle);
	if (status < 0){
        printf("Failed to Initialize Accelerometer -> Exiting program\n");
		printf("Check i2cdetect -r -y 1 for 0x6B\n");
		mraa_i2c_stop(I2Chandle);
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

	Gyroscope_Init();
	ReadSensorValues();
	
	return 0;
}

