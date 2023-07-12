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
 * FILE NAME  :  M1_L3_E3a_I2C_LM75.c      
 * DESCRIPTION:  This program outputs Temperature value
 *               Turns OFF LM75A when button 2 is pressed
 *               Turns ON LM75A when button 1 is pressed
 * *************************************************/

 #include "mraa_beaglebone_pinmap.h"

#define LM75A_I2C_ADDR        0x48


int main(int argc, char** argv)
{
	float T = 0;
	unsigned int x = 0, B1_val =0, B2_val = 0;
	unsigned char buf [2] = {0, 0}, shutdown[2] = {0, 1};

	mraa_init();
	mraa_gpio_context button1;
	mraa_gpio_context button2;
	button1 = mraa_gpio_init(B1);
	button2 = mraa_gpio_init(B2);

	mraa_i2c_context i2cs;
	i2cs = mraa_i2c_init_raw (I2CS_BUS);	
	mraa_i2c_frequency (i2cs, MRAA_I2C_STD);
	mraa_i2c_address(i2cs, LM75A_I2C_ADDR);



	while (1)
	{
		mraa_i2c_read(i2cs, buf, 2);
		B1_val = mraa_gpio_read(button1);
		B2_val = mraa_gpio_read(button2);
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
		sleep(1);

		if(!B1_val)  
			mraa_i2c_write (i2cs, shutdown, 1);
		if(!B2_val)    
			mraa_i2c_write (i2cs, shutdown+1, 1);
	}

}
