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
 * FILE NAME  :  button.c      
 * DESCRIPTION:  This code reads the input from B4 
 *               and returns the corresponding value.
 * *************************************************/

#include "mraa_beaglebone_pinmap.h"

mraa_gpio_context pb2;

int main(int argc, char** argv)
{
	int x;
	mraa_init();

	pb2 = mraa_gpio_init(B4);
	mraa_gpio_dir(pb2, MRAA_GPIO_IN);

	if (mraa_gpio_read (pb2) == 0)
	{
		return 1;
	}

	return 0;
}
