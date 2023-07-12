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
 * FILE NAME  :  M1_L1_AccelTestDisplay.c
 * DESCRIPTION:  This program reads acceleration and 
 *               Displays the X,Y, and Z acceliration
 *               valuse on the LCD.
 *
 **************************************************/

#include "mraa_beaglebone_pinmap.h"
#include <time.h>
#define LCD_ADDR 		 0x3E
// check the clave address : as of now keeping SA0 =1
#define I2C_SLAVE_ADDR 0x6B

#define CTRL_REG5_XL			0x1F
#define CTRL_REG6_XL			0x20
#define CTRL_REG7_XL			0x21
#define CTRL_REG9           	0x23

#define FIFO_CTRL				0x2E
#define FIFO_SRC				0x2F
#define OUT_X_L_XL           	0x28

// Sensor Sensitivity Constants
// Values set according to the typical specifications provided in
// table 3 of the LSM9DS1 datasheet. (pg 12)
#define SENSITIVITY_ACCELEROMETER_2  0.000061
#define SENSITIVITY_ACCELEROMETER_4  0.000122
#define SENSITIVITY_ACCELEROMETER_8  0.000244
#define SENSITIVITY_ACCELEROMETER_16 0.000732


mraa_i2c_context I2Chandle;
float aRes;
float AccelVal[3];
int16_t aRaw[3];
int16_t ax, ay, az; // x, y, and z axis readings of the Accelerometer

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


void initI2C(void)  // sets up both I2C Busses
{
	mraa_init();
//Initialize Display i2c Bus
    i2cp = mraa_i2c_init_raw (I2CP_BUS);
	mraa_i2c_frequency (i2cp, MRAA_I2C_STD);
	mraa_i2c_address(i2cp, LCD_ADDR);

    I2Chandle = mraa_i2c_init_raw (I2CS_BUS); // write correct I2C dev number here
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

void ReadAccelValues()
{
	int i;
	float sample;
	char buf [20];

	uint8_t tempBuffer[6]; // We'll read six bytes from the accel into temp
	int32_t AccelRawData[3] = {0, 0, 0}, ii;
	
	for (i = 0; i < 2000; i ++)
	{
		AccelRawData[0] =0;
		AccelRawData[1] =0;
		AccelRawData[2] =0; 
		ax,ay,az=0;

		mraa_i2c_read_bytes_data(I2Chandle, OUT_X_L_XL, tempBuffer, 6) ;// Read 6 bytes, beginning at OUT_X_L_XL

		ax = (tempBuffer[1] << 8) | tempBuffer[0]; // Store x-axis values into ax
		ay = (tempBuffer[3] << 8) | tempBuffer[2]; // Store y-axis values into ay
		az = (tempBuffer[5] << 8) | tempBuffer[4]; // Store z-axis values into az
			
		AccelRawData[0] = ax;
		AccelRawData[1] = ay;
		AccelRawData[2] = az;// Assumes sensor facing up!
  
		for (ii = 0; ii < 3; ii++)
		{
			AccelVal[ii] = aRes * AccelRawData[ii]; //Now the value is in g i.e m/s2
		}
		
		sprintf (buf, " X     Y    Z ");
		LCD_Print ((uint8_t*)buf);
		sprintf(buf,"%3.2f %3.2f %3.2f   ", AccelVal[0], AccelVal[1], AccelVal[2]);
		LCD_Print2 (buf);
		printf(" \n Acceleration in :\n X direction = %3.2f\t Y direction = %3.2f\t Z direction = %3.2f g\n", AccelVal[0], AccelVal[1], AccelVal[2]);
		sleep (1);
	}
	
	
		
}

int main(void) {
   	int status; 
    	initI2C();
	clear_LCD();
	Accelerometer_Init();
	status = mraa_i2c_read_byte(I2Chandle);
	 if (status < 0){
              printf("Failed to Initialize Accelerometer -> Exiting program\n");
			  printf("Check i2cdetect -r -y 1 for 0x6B\n");
              mraa_i2c_stop(I2Chandle);
              mraa_deinit();
              return EXIT_FAILURE;
             }
	ReadAccelValues();
	return 0;
}

