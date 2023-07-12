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
 * FILE NAME  :  M1_L3_E3c_I2C_multiple.c      
 * DESCRIPTION:  This program outputs Temperature value
 *               read from LM75A and Pressure value read 
 *				 from MPL3115A2 in a loop.
 * *************************************************/

#include "mraa_beaglebone_pinmap.h"
#include <time.h>
#include <stdbool.h>

#define LM75A_I2C_ADDR        0x48

#define PRESSURE_I2C_ADDR     0x60
#define STATUS     0x00
#define OUT_P_MSB  0x01
#define OUT_P_CSB  0x02
#define OUT_P_LSB  0x03
#define CTRL_REG1  0x26
#define PT_DATA_CFG 0x13

#define LCD_ADDR 		 0x3e

mraa_i2c_context i2cs;
mraa_i2c_context i2cp;

void toggleOneShot(void)
{

	unsigned char Value = mraa_i2c_read_byte_data(i2cs, CTRL_REG1); //Read current settings
	Value &= ~(1 << 1); //Clear OST bit
	mraa_i2c_write_byte_data(i2cs, Value, CTRL_REG1);

	Value = mraa_i2c_read_byte_data(i2cs, CTRL_REG1); //Read current settings to be safe
	Value |= (1 << 1); //Set OST bit
	mraa_i2c_write_byte_data(i2cs, Value, CTRL_REG1);
}


float ReadPressure(void)
{
	unsigned char msb =0, csb=0, lsb=0;
		
	if (mraa_i2c_read_byte_data(i2cs, STATUS) & (1 << 2) == 0) 
	{
		toggleOneShot();					//Toggle the OST bit causing the sensor to immediately take another reading
	}

	// Wait for PDR bit, indicates we have new pressure data
	while (mraa_i2c_read_byte_data(i2cs, STATUS) & (1 << 2) == 0)
	{
		usleep(10);
	}

	
	msb = mraa_i2c_read_byte_data(i2cs, OUT_P_MSB);
	csb = mraa_i2c_read_byte_data(i2cs, OUT_P_CSB);
	lsb = mraa_i2c_read_byte_data(i2cs, OUT_P_LSB);
	
	toggleOneShot(); //Toggle the OST bit causing the sensor to immediately take another reading

	// Pressure comes back as a left shifted 20 bit number
	long pressure_whole = (long)msb << 16 | (long)csb << 8 | (long)lsb;
	pressure_whole >>= 6;

	lsb &= 0x30; //Bits 5/4 represent the fractional component
	lsb >>= 4; //Get it right aligned
	float pressure_decimal = (float)lsb / 4.0; //Turn it into fraction

	float pressure = (float)pressure_whole + pressure_decimal;

	return(pressure);


}

void SetupPressureSensor(void)
{
	unsigned char tempVal;
	
	mraa_i2c_write_byte_data(i2cs, 0x07, PT_DATA_CFG); // Enable all three pressure and temp event flags 
	
	tempVal =  mraa_i2c_read_byte_data(i2cs, CTRL_REG1);
	
	// Activate Barometer mode 
	tempVal &= ~(1<<7); 		//Clear ALT bit
	tempVal |= (1<<0); 			//Set SBYB bit for Active mode
	mraa_i2c_write_byte_data(i2cs, tempVal, CTRL_REG1);
	
}

void home_LCD (void)
{
	uint8_t buf[2] = {0x00,0x02};
	mraa_i2c_write(i2cp, buf, 2);  //Set to Home
}

void LCD_Print (char* str)
{
	uint8_t buf[80];

	int32_t i = 0, strl, j=0;

	buf[i] = 0x40;  //register for display
	strl = strlen((char*)str);
	for (j = 0; j < strl; j++)
	{
		i++;
		buf[i] = str[j];
	}

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

void clear_LCD (void)
{

	uint8_t buf[2] = {0x00,0x01};

	mraa_i2c_write(i2cp, buf, 2);  //Clear Display
}

void nextline_LCD (void)
{
	uint8_t buf[2] = {0x00,0xC0};

	mraa_i2c_write(i2cp, buf, 2);  //2nd line of LCD Display
}

int main(int argc, char** argv)
{
	float T = 0;
	float pressure =0;
	unsigned int x = 0, B1_val =0, B2_val = 0, B3_val = 0;
	unsigned char buf [2] = {0, 0};
	bool readTemp = true, readPressure = true;
	char lcdString[16];

	mraa_init();
	
	mraa_gpio_context button1;
	mraa_gpio_context button2;
	mraa_gpio_context button3;
	button1 = mraa_gpio_init(B1);
	button2 = mraa_gpio_init(B2);
	button3 = mraa_gpio_init(B3);
	
	i2cs = mraa_i2c_init_raw (I2CS_BUS);	
	mraa_i2c_frequency (i2cs, MRAA_I2C_STD);
	mraa_i2c_address(i2cs, PRESSURE_I2C_ADDR);
	
	i2cp = mraa_i2c_init_raw (I2CP_BUS);
	mraa_i2c_frequency (i2cp, MRAA_I2C_STD);
	mraa_i2c_address(i2cp, LCD_ADDR);
	
	SetupPressureSensor();
	
	LCD_init ();
	LCD_Print ((char*)"Keysight U3810A");

	while (1)
	{
		// Check buttons
		B1_val = mraa_gpio_read(button1);
		B2_val = mraa_gpio_read(button2);
		B3_val = mraa_gpio_read(button3);
		
		if (!B1_val)
		{
			readTemp = true;
			readPressure = true;
		}
		else if (!B2_val)
		{
			readTemp = true;
			readPressure = false;
		}
		else if (!B3_val)
		{
			readTemp = false;
			readPressure = true;
		}
		
		clear_LCD();
		home_LCD();
		
		// Read temperature sensor
		if (readTemp)
		{
			mraa_i2c_address(i2cs, LM75A_I2C_ADDR);
			mraa_i2c_read(i2cs, buf, 2);

			x = (buf [0] << 8) | buf [1];
			x = (x & 0x7FFF) >> 7;
			T = x / 2.0;
			if (buf [0] & 0x80)
			{
				sprintf(lcdString, "-%0.1f degC", T);
			}
			else
			{
				sprintf(lcdString, "+%0.1f degC", T);
			}
			
			LCD_Print(lcdString);
			nextline_LCD();
		}
		
		// Read pressure sensor
		if (readPressure)
		{
			mraa_i2c_address(i2cs, PRESSURE_I2C_ADDR);
			pressure = ReadPressure();
			pressure = pressure / 1000;
			sprintf(lcdString, "%0.1f kPa", pressure);
			LCD_Print(lcdString);
		}
		
		sleep (1);
	}
}

