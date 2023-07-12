/*************************************************
 *         Keysight Technologies 2019 
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
 * FILE NAME  :  M2_L2_Appendix_print2lcd.c      
 * DESCRIPTION:  This program converts the raw values 
 *               passed, to temperature and dsiplays
 * *************************************************/
#include "mraa_beaglebone_pinmap.h"

#define LCD_ADDR 		 0x3e

// mraa gpio context declaration
mraa_i2c_context i2cp;

//U3810A LCD Functions
void home_LCD (void)
{
	uint8_t buf[2] = {0x00,0x02};
	mraa_i2c_write(i2cp, buf, 2);  //Set to Home
}

void LCD_Print (char* str)
{
	uint8_t buf[80];
	uint8_t buf1[2]={0x00,0x80};

	int32_t i = 0, strl, j=0;

	buf[i] = 0x40;  //register for display
	strl = strlen((char*)str);
	for (j = 0; j < strl; j++)
	{
		i++;
		buf[i] = str[j];
	}
	mraa_i2c_write(i2cp, buf1, 2);
	mraa_i2c_write(i2cp, buf, i+1);
}

void LCD_init (void)
{
	uint8_t init1[2] = {0x00,0x38};
	uint8_t init2[8] = {0x00, 0x39, 0x14,0x74,0x54,0x6f,0x0c,0x01};
	// 2 lines 8 bit 3.3V Version
	mraa_i2c_write(i2cp, init1, 2);
	mraa_i2c_write(i2cp, init2,8);  //Function Set
}

// Convert humidity sensor raw data to temperature in degree celsius and humidity in relative humidity
void sensorHdc1000Convert(uint16_t rawTemp, uint16_t rawHum, float *temp, float *hum)
{
	*temp = ((double)(int16_t)rawTemp / 65536)*165 - 40;
	rawHum &= ~0x0003;
	*hum = ((double)rawHum / 65536)*100;
}

int main(int argc, char** argv)
{
	char temp2[15] = {0};
    	long rawTempData = strtol(argv[1], NULL, 16);
    	long rawHumData = strtol(argv[2], NULL, 16);
    	uint16_t rawTemp, rawHum;
    	float temp, hum;

    	//Initialise gpio_context
	mraa_init();

	//Initialise i2c context, passing in the i2c bus to use
	i2cp = mraa_i2c_init_raw (I2CP_BUS);

	//Sets the frequency of the i2c context.
	mraa_i2c_frequency (i2cp, MRAA_I2C_STD);
	
	//Sets the i2c slave address
	mraa_i2c_address(i2cp, LCD_ADDR);

	rawTemp = ((rawTempData<<8) & 0xff00) | ((rawTempData>>8) & 0x00ff);
    	rawHum = ((rawHumData<<8) & 0xff00) | ((rawHumData>>8) & 0x00ff);
    	sensorHdc1000Convert (rawTemp, rawHum, &temp, &hum);

	// Print temperature data in terminal
	printf("Temp: %.1f C\n", temp);

	// Display temperature data on LCD
	sprintf(temp2, "%s %.1f %c", "temp:", temp, 'C');
	LCD_init ();
	LCD_Print (temp2);



	return 0;
}

