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
 * FILE NAME  :  M1_L3_T3b_I2C_multiple.c      
 * DESCRIPTION:  This program outputs Temperature value
 *               read from LM75A and Pressure value read 
 *				 from MPL3115A2 in a loop.
 * *************************************************/

#include "mraa_beaglebone_pinmap.h"
#include <time.h>

#define LM75A_I2C_ADDR        0x48

#define PRESSURE_I2C_ADDR     0x60
#define STATUS     0x00
#define OUT_P_MSB  0x01
#define OUT_P_CSB  0x02
#define OUT_P_LSB  0x03
#define CTRL_REG1  0x26
#define PT_DATA_CFG 0x13

mraa_i2c_context i2cs;

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

int main(int argc, char** argv)
{
	float T = 0;
	float pressure =0;
	unsigned int x = 0;
	unsigned char buf [2] = {0, 0};

	mraa_init();
	
	i2cs = mraa_i2c_init_raw (I2CS_BUS);	
	mraa_i2c_frequency (i2cs, MRAA_I2C_STD);
	mraa_i2c_address(i2cs, PRESSURE_I2C_ADDR);
	
	SetupPressureSensor();

	while (1)
	{
		// Read temperature sensor
		mraa_i2c_address(i2cs, LM75A_I2C_ADDR);
		mraa_i2c_read(i2cs, buf, 2);

		x = (buf [0] << 8) | buf [1];
		x = (x & 0x7FFF) >> 7;
		T = x / 2.0;
		if (buf [0] & 0x80)
		{
			printf("-%0.1f degC\n", T);
		}
		else
		{
			printf("+%0.1f degC\n", T);
		}
		sleep (1);
		
		// Read pressure sensor
		mraa_i2c_address(i2cs, PRESSURE_I2C_ADDR);
		pressure = ReadPressure();
		pressure = pressure / 1000;
		printf("%0.1f kPa\n", pressure);
		
		sleep (1);
	}
}

