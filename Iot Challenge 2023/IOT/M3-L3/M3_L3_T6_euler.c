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
 * FILE NAME  :  M3-L3-euler.c
 * DESCRIPTION:  This program reads Gyroscope, Accelerometer values and integrates them and 
 *               Displays the X,Y, and Z angle
 *               values on the LCD.
 *
 **************************************************/

#include "mraa_beaglebone_pinmap.h"
#include <time.h>
#include <math.h>
#include <stdbool.h>

#define LCD_ADDR 		 0x3E
// check the clave address : as of now keeping SA0 =1
#define I2C_SLAVE_ADDR 		0x6B
#define I2C_SLAVE_ADDR_MAG 	0x1E

#define CTRL_REG1_G			0x10
#define CTRL_REG2_G			0x11
#define CTRL_REG3_G			0x12
#define CTRL_REG4           0x1E
#define CTRL_REG9           0x23
#define CTRL_REG5_XL		0x1F
#define CTRL_REG6_XL		0x20
#define CTRL_REG7_XL		0x21
#define CTRL_REG1_M		0x20
#define CTRL_REG2_M		0x21
#define CTRL_REG3_M		0x22
#define CTRL_REG4_M		0x23
#define CTRL_REG5_M		0x24

#define FIFO_CTRL			0x2E
#define FIFO_SRC			0x2F
#define OUT_X_L_G           0x18
#define OUT_X_L_XL           0x28
#define OUT_X_L_M	0x28

#define settings_MAG_enabled 	true
#define	settings_MAG_scale	4
#define	settings_MAG_sampleRate 	7
#define	settings_MAG_tempCompensationEnable	false
#define	settings_MAG_XYPerformance 3
#define	settings_MAG_ZPerformance 3
#define	settings_MAG_lowPowerEnable false
#define	settings_MAG_operatingMode 0

#define SENSITIVITY_GYROSCOPE_245    0.00875
#define SENSITIVITY_GYROSCOPE_500    0.0175
#define SENSITIVITY_GYROSCOPE_2000 	 0.07

#define SENSITIVITY_ACCELEROMETER_2   0.000061
#define SENSITIVITY_ACCELEROMETER_4   0.000122
#define SENSITIVITY_ACCELEROMETER_8   0.000244
#define SENSITIVITY_ACCELEROMETER_16   0.000732

#define SENSITIVITY_MAGNETIC_2	0.00014
#define SENSITIVITY_MAGNETIC_4	0.00029
#define SENSITIVITY_MAGNETIC_8	0.00043
#define SENSITIVITY_MAGNETIC_16	0.00058

#define DELTA_T  0.01
#define COEFF    0.98
#define RAD_TO_DEG 180 / M_PI

#define BILLION 1E9

mraa_i2c_context I2Chandle;
mraa_i2c_context I2Chandle_mag;
mraa_uart_context UARThandle;

float gRes, aRes, mRes;
float GyroVal[3], AccVal[3], MagVal[3];
int16_t gx, gy, gz, ax, ay, az, mx, my, mz; // x, y, and z axis readings of the gyroscope
float pitch, roll, yaw;
float heading, headingDegrees, headingFiltered, declination;


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


void calcAngle() {
	float measAngleP = pitch +  (GyroVal[0] * DELTA_T);
	float measAngleR = roll +  (GyroVal[1] * DELTA_T);
	float measAngleY = yaw +  (GyroVal[2] * DELTA_T);

	// pitch = (measAngleP * COEFF) + (atan2f(AccVal[0], sqrt(AccVal[1]*AccVal[1] + AccVal[2]*AccVal[2])) * RAD_TO_DEG) * (float)(1 - COEFF);
	// roll = (measAngleR * COEFF) + (atan2f(AccVal[1], sqrt(AccVal[0]*AccVal[0] + AccVal[2]*AccVal[2])) * RAD_TO_DEG) * (float)(1 - COEFF);
	// yaw = (measAngleY * COEFF) + (atan2f(AccVal[2], sqrt(AccVal[0]*AccVal[0] + AccVal[1]*AccVal[1])) * RAD_TO_DEG) * (float)(1 - COEFF);

	heading = atan2f(MagVal[0], MagVal[1]);
	declination = 0.073;
	heading = heading + declination;
    if(heading<0){
		heading = heading + (2 * M_PI);
	} 
	else if(heading > 2 * M_PI) {
		heading = heading - (2 * M_PI);
	}
	headingDegrees = heading * RAD_TO_DEG;
	if(headingDegrees<=180) {
		headingDegrees = headingDegrees + 180;
	} else if (headingDegrees > 180) {
 	headingDegrees = headingDegrees - 180;
	}
	headingFiltered = headingFiltered*0.85+headingDegrees*0.15;

	 pitch = (measAngleP * COEFF) + (atan2f(AccVal[1],AccVal[2]) * RAD_TO_DEG) * (float)(1 - COEFF);
	 roll = (measAngleR * COEFF) + (atan2f(AccVal[0],AccVal[2]) * RAD_TO_DEG) * (float)(1 - COEFF);
	 yaw = headingDegrees;

	//pitch = GyroVal[0];
	//roll = GyroVal[1];
	//yaw = GyroVal[2];

}

void ReadSensorValues()
{
	char buf[20];
	int ret;
	while(1){
		ReadGyro();
		ReadAccel();
		ReadMagneto();
		calcAngle();
		// printf("Gyr:,%3.3f, %3.3f, %3.3f\n", GyroVal[0], GyroVal[1], GyroVal[2]);
		// printf("Acc:,%3.3f, %3.3f, %3.3f\n", AccVal[0], AccVal[1], AccVal[2]);
		// printf("Mag:,%3.2f, %3.2f, %3.2f, %6.2f, %6.2f \n", MagVal[0], MagVal[1], MagVal[2], heading, headingDegrees);
		// printf("Orientation:,%6.3f,%6.3f,%6.3f\n", yaw, pitch, roll);
		printf("%5.0f,%5.0f,%5.0f\n", yaw, pitch, roll);
		//sprintf(buf, "%6.3f %6.3f %6.3f", yaw, pitch, roll);
		//ret = mraa_uart_write(UARThandle, buf, 20);
		//printf("return %d\n", ret);
		//usleep (2250);
		//usleep(1000);
	}
	
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

	I2Chandle_mag = mraa_i2c_init_raw (I2CS_BUS); // write correct I2C dev number here
	if (!I2Chandle_mag) { // The bus is not there 
		printf("I2C Bus 1 is not available. Please check /dev/i2c-2 \n");
		printf(" -> Exiting program\n");
		mraa_i2c_stop(I2Chandle_mag);
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


	pitch = 0.0;
	roll = 0.0;
	yaw = 0.0;
	headingFiltered = 0.0;
		
	Gyroscope_Init();
	Accelerometer_Init();
	Magnetometer_Init();
	ReadSensorValues();
	

	return 0;
}

