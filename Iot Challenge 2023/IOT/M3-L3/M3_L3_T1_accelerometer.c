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
 * FILE NAME  :  M3_L3_T1_accelerometer.c
 * DESCRIPTION:  This program reads Accelerometer values and output to a CSV file.
 * 
 *
 **************************************************/

#include "mraa_beaglebone_pinmap.h"
#include <time.h>
#include <math.h>
#define LCD_ADDR 		 0x3E
// check the clave address : as of now keeping SA0 =1
#define I2C_SLAVE_ADDR 		0x6B
#define CTRL_REG1_G			0x10
#define CTRL_REG2_G			0x11
#define CTRL_REG3_G			0x12
#define CTRL_REG4           0x1E
#define CTRL_REG9           0x23
#define CTRL_REG5_XL		0x1F
#define CTRL_REG6_XL		0x20
#define CTRL_REG7_XL		0x21
#define FIFO_CTRL			0x2E
#define FIFO_SRC			0x2F
#define OUT_X_L_XL          0x28
#define SENSITIVITY_ACCELEROMETER_2   0.000061
#define SENSITIVITY_ACCELEROMETER_4   0.000122
#define SENSITIVITY_ACCELEROMETER_8   0.000244
#define SENSITIVITY_ACCELEROMETER_16   0.000732
#define DELTA_T  0.01
#define COEFF    0.98
#define RAD_TO_DEG 180 / M_PI

#define BILLION 1E9

mraa_i2c_context I2Chandle;
mraa_uart_context UARThandle;

float aRes;
float AccVal[3];
int16_t ax, ay, az; 
struct timespec start, stop;
float time_elapse;
FILE *fp;

void Accelerometer_Init()
{
    // Accelerometer settings
    uint8_t tempRegValue = 0;
    uint8_t sampleRate,highResEnable;
    uint8_t bandwidth;
    uint8_t scale;
    
    //	CTRL_REG5_XL (0x1F) (Default value: 0x38): [DEC_1][DEC_0][Zen_XL][Yen_XL][Zen_XL][0][0][0]
	
	tempRegValue |= (1<<5);	
	tempRegValue |= (1<<4);
	tempRegValue |= (1<<3);
	
	mraa_i2c_write_byte_data(I2Chandle,tempRegValue,CTRL_REG5_XL);
	
	// CTRL_REG6_XL (0x20) (Default value: 0x00) :[ODR_XL2][ODR_XL1][ODR_XL0][FS1_XL][FS0_XL][BW_SCAL_ODR][BW_XL1][BW_XL0]
	
	tempRegValue = 0;
	// accel sample rate can be 1-6
	// 1 = 10 Hz    4 = 238 Hz
	// 2 = 50 Hz    5 = 476 Hz
	// 3 = 119 Hz   6 = 952 Hz
	sampleRate = 6;

	tempRegValue |= (sampleRate & 0x07) << 5;
	
	// accel scale can be 2, 4, 8, or 16
	scale = 16;
	aRes = SENSITIVITY_ACCELEROMETER_16; // Calculate g / ADC tick, stored in aRes variable
	
	
	if(scale ==  4)
			tempRegValue |= (0x2 << 3);
	else if (scale == 8)
			tempRegValue |= (0x3 << 3);
	else if (scale == 16)
			tempRegValue |= (0x1 << 3);
	else
			tempRegValue |= (0x0 << 3); // Otherwise it'll be set to 2g (0x0 << 3)
	// Accel cutoff freqeuncy can be any value between -1 - 3. 
	// -1 = bandwidth determined by sample rate
	// 0 = 408 Hz   2 = 105 Hz
	// 1 = 211 Hz   3 = 50 Hz
	bandwidth = -1;
	if (bandwidth >= 0)
	{
		tempRegValue |= (1<<2); // Set BW_SCAL_ODR
		tempRegValue |= (bandwidth & 0x03);
	}
	
	mraa_i2c_write_byte_data(I2Chandle,tempRegValue,CTRL_REG6_XL);
	
	// CTRL_REG7_XL (0x21) (Default value: 0x00): [HR][DCF1][DCF0][0][0][FDS][0][HPIS1]
	tempRegValue = 0;
	highResEnable = 0;
	if (highResEnable)
	{
		//tempRegValue |= (1<<7); // Set HR bit
		//tempRegValue |= (highResBandwidth & 0x3) << 5;
	}
	
	mraa_i2c_write_byte_data(I2Chandle,tempRegValue,CTRL_REG7_XL);
}

void ReadAccel() {
	uint8_t tempBuffer[6];

	mraa_i2c_read_bytes_data(I2Chandle, OUT_X_L_XL, tempBuffer, 6);

	ax = (tempBuffer[1] << 8) | tempBuffer[0]; // Store x-axis values into gx
	ay = (tempBuffer[3] << 8) | tempBuffer[2]; // Store y-axis values into gy
	az = (tempBuffer[5] << 8) | tempBuffer[4]; // Store z-axis values into gz

	AccVal[0] = aRes * ax;
	AccVal[1] = aRes * ay;
	AccVal[2] = aRes * az;
}

void ReadSensorValues()
{
    fp = fopen("accelresults.csv","w");

    char buf[20];
	int ret;
    int i;


	clock_gettime (CLOCK_REALTIME, &start);
	while(1){
		ReadAccel();
        fprintf(fp, "%5.5f, %5.5f, %5.5f\n", AccVal[0], AccVal[1], AccVal[2]);
        printf("%5.5f, %5.5f, %5.5f\n", AccVal[0], AccVal[1], AccVal[2]);
	}
	// clock_gettime (CLOCK_REALTIME, &stop);
	// time_elapse = (stop.tv_sec + (float) stop.tv_nsec / BILLION) - (start.tv_sec + (float)start.tv_nsec / BILLION);
    // printf("Total time elapse: %f seconds\n", time_elapse);

    // clock_gettime (CLOCK_REALTIME, &start);
    // for(i=0; i < 2000; i++) {
    //     ReadAccel();
    //     fprintf(fp, "%5.5f, %5.5f, %5.5f\n", AccVal[0], AccVal[1], AccVal[2]);
    //     printf("%5.5f, %5.5f, %5.5f\n", AccVal[0], AccVal[1], AccVal[2]);
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

	Accelerometer_Init();
	ReadSensorValues();
	
	return 0;
}

