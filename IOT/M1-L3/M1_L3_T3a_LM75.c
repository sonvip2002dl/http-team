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
 * FILE NAME  :  M1_L3_T3a_LM75.c      
 * DESCRIPTION:  This program outputs Temperature value
 *               read from LM75A in loop.
 * *************************************************/

#include "mraa_beaglebone_pinmap.h"

#define LM75A_I2C_ADDR        0x48


int main(int argc, char** argv)
{
	float T = 0;
	unsigned int x = 0;
	unsigned char buf [2] = {0, 0};

	mraa_init();

	mraa_i2c_context i2cs;
	i2cs = mraa_i2c_init_raw (I2CS_BUS);	
	mraa_i2c_frequency (i2cs, MRAA_I2C_STD);
	mraa_i2c_address(i2cs, LM75A_I2C_ADDR);

	while (1)
	{

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
	}
}

