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
* FILE NAME  :  M1_A1_T2_light.c      
* DESCRIPTION:  This code switches relay ON or OFF.
*
* *************************************************/

#include <stdio.h>
#include "mraa_beaglebone_pinmap.h"

int main(int argc, char** argv)
{
	mraa_init();
	
	// create a GPIO object from MRAA using it
	mraa_gpio_context relay;
	relay = mraa_gpio_init(RELAY_PIN);
	mraa_gpio_dir(relay, MRAA_GPIO_OUT);

	// toggle relay if input is true
	if (strstr (argv[1],"1"))
	{
		mraa_gpio_write(relay, 0);
		printf("Switch ON\n");
	}
	else
	{
		mraa_gpio_write(relay, 1);
		printf("Switch OFF\n");
	}

	return MRAA_SUCCESS;
}
