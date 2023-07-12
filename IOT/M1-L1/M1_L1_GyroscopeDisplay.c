/*************************************************
 *         (C) Keysight Technologies 2020 
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
 * FILE NAME  :  M1_L1_GyroscopeDisplay.c
 * DESCRIPTION:  This program reads Gyroscope values,
 *				 integrates them and displays
 *               the X,Y and Z angle
 *               values on the LCD.
 *
 **************************************************/

#include "mraa_beaglebone_pinmap.h"
#include <time.h>
#include <math.h>
#define LCD_ADDR 		 0x3E
// check the clave address : as of now keeping SA0 =1
#define I2C_SLAVE_ADDR 0x6B

#define CTRL_REG1_G			0x10
#define CTRL_REG2_G			0x11
#define CTRL_REG3_G			0x12
#define CTRL_REG4           0x1E
#define CTRL_REG9           0x23
#define CTRL_REG6_XL		0x20

#define FIFO_CTRL			0x2E
#define FIFO_SRC			0x2F
#define OUT_X_L_G           0x18

#define SENSITIVITY_GYROSCOPE_245    0.00875
#define SENSITIVITY_GYROSCOPE_500    0.0175
#define SENSITIVITY_GYROSCOPE_2000 	 0.07


mraa_i2c_context I2Chandle;
float gRes;
float GyroVal[3];
float GyroInteg[3] = {0.0,0.0,0.0};
int16_t gRaw[3];
int16_t gx, gy, gz; // x, y, and z axis readings of the gyroscope

mraa_i2c_context i2cp;


void home_LCD (void)
{
        uint8_t buf[2] = {0x00,0x02};
        mraa_i2c_write(i2cp, buf, 2);  //Set to Home
}
void home2_LCD (void)
{
        uint8_t buf[] = {0x00,0x02,0xC0};
        mraa_i2c_write(i2cp, buf, 3);  //Set to Start of 2nd line 0X40 
}

void LCD_Print2 (char* str)
{
        uint8_t buf[80]={0};  // Set Buffer to all Null
        int i = 0, strl;      
        home2_LCD ();
        buf[i] = 0x40;  //register for display
        i++;
        strl = strlen((char*)str);
        for (int j = 0; j < strl; j++)
        {
               buf[i] = str[j];
               i++;

        }
         mraa_i2c_write(i2cp, buf, i);
}
void LCD_Print (uint8_t* str)
{
        uint8_t buf[80]={0};   // Set Buffer to all Null
	uint8_t buf1[2]={0x00,0x80};	
        int32_t i = 0, strl, j=0; 
        buf[i] = 0x40;  //register for display
        i++;
        strl = strlen((char*)str);
        for (j = 0; j < strl; j++)
        {
                buf[i] = str[j];
		i++;
  
        }
        
	mraa_i2c_write(i2cp, buf1, 2);
        mraa_i2c_write(i2cp, buf, i);
}

void LCD_init (void)
{
   uint8_t init1[2] = {0x00,0x38};
   uint8_t init2[8] = {0x00, 0x39, 0x14,0x74,0x54,0x6f,0x0c,0x01};
   // 2 lines 8 bit 3.3V Version
    mraa_i2c_write(i2cp, init1, 2);
    mraa_i2c_write(i2cp, init2,8);  //Function Set
}

void clear_LCD (void)
{
        uint8_t buf[2] = {0x00,0x01};
        mraa_i2c_write(i2cp, buf, 2);  //Clear Display
}

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
void initI2C(void)
{
	I2Chandle = mraa_i2c_init_raw (1); // write correct I2C dev number here
	mraa_i2c_frequency (I2Chandle, MRAA_I2C_STD);
	mraa_i2c_address(I2Chandle, I2C_SLAVE_ADDR);

}

void enableFIFO(int enable)
{
	uint8_t temp = mraa_i2c_read_byte_data(I2Chandle,CTRL_REG9);

	if (enable)
		temp |= (1<<1);
	else 
		temp &= ~(1<<1);

	mraa_i2c_write_byte_data(I2Chandle, temp,CTRL_REG9);
	mraa_i2c_write_byte_data(I2Chandle, 0xDF, FIFO_CTRL); //set threshold to 32 samples
	
}

void ReadGyroValues()
{
	uint8_t tempBuffer[6]; // We'll read six bytes from the gyro into temp
	uint8_t data[6] = {0, 0, 0, 0, 0, 0};
	uint8_t samples = 0;
	int ii;
	int32_t GyroRawData[3] = {0, 0, 0};
	int i;
	float sample;
	char buf [20];

	// Turn on FIFO and set threshold to 32 samples
	enableFIFO(1);
	for (i = 0; i < 20000; i ++)
	{
		GyroRawData[0] =0;
		GyroRawData[1] =0;
		GyroRawData[2] =0;
		while (samples < 0x1F)
		{
			samples = (mraa_i2c_read_byte_data(I2Chandle,FIFO_SRC )& 0x3F); // Read number of stored samples
		}
		

		for(ii = 0; ii < samples ; ii++) 
		{	// Read the gyro data stored in the FIFO
			mraa_i2c_read_bytes_data(I2Chandle, OUT_X_L_G, tempBuffer, 6); // Read 6 bytes, beginning at OUT_X_L_G

			gx = (tempBuffer[1] << 8) | tempBuffer[0]; // Store x-axis values into gx
			gy = (tempBuffer[3] << 8) | tempBuffer[2]; // Store y-axis values into gy
			gz = (tempBuffer[5] << 8) | tempBuffer[4]; // Store z-axis values into gz

			GyroRawData[0] += gx;
			GyroRawData[1] += gy;
			GyroRawData[2] += gz;
		}  
		for (ii = 0; ii < 3; ii++)
		{
			gRaw[ii] = GyroRawData[ii] / samples;
			GyroVal[ii] = gRes * gRaw[ii];
			if (fabs(GyroVal[ii]) > 0.2) {
				GyroInteg[ii] += GyroVal[ii]/20;
			}


		}
				sprintf (buf, " X     Y    Z ");
		LCD_Print ((uint8_t*)buf);
		sprintf(buf,"%3.0f %3.0f %3.0f   ", GyroInteg[0], GyroInteg[1], GyroInteg[2]);
		LCD_Print2 (buf);
		//printf(" \n Rotations on :\n X axis = %f\t Y axis = %f\t Z axis = %f\n", GyroVal[0], GyroVal[1], GyroVal[2]);
		usleep (2250);
	}
	
	enableFIFO(0);

}

int main(void) {
  	int status; 
	// Initialize and check the I2C Display
	i2cp = mraa_i2c_init_raw (I2CP_BUS);
	if (!i2cp) { // The bus is not there 
		printf("I2C Bus 2 is not available. Please check /dev/i2c-2 \n");
		printf(" -> Exiting program\n");
		mraa_i2c_stop(i2cp);
		mraa_deinit();
		return EXIT_FAILURE;
	}

	mraa_i2c_frequency (i2cp, MRAA_I2C_STD);
	mraa_i2c_address(i2cp, LCD_ADDR);
	status=mraa_i2c_read_byte(i2cp);
	if (status < 0){
         printf("Failed to initialize display\n");
		 printf("Check i2cdetect -r -y 2 for 0x3E\n");
         mraa_i2c_stop(i2cp);
         mraa_deinit();
         return EXIT_FAILURE;
        }
		
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
		
	Gyroscope_Init();
	clear_LCD();
	ReadGyroValues();
	

	return 0;
}

