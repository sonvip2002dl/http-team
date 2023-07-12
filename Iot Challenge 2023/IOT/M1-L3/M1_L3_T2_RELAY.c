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
* FILE NAME  :  M1_L3_T2_RELAY.c      
* DESCRIPTION:  This code turns Relay ON/OFF in loop.
*
* *************************************************/

#include "mraa_beaglebone_pinmap.h"

int main()
{
	mraa_init();
	
	// create a GPIO object from MRAA using it
	mraa_gpio_context relay;
	relay = mraa_gpio_init(RELAY_PIN);
	
	if (relay == NULL) 
	{
		printf("MRAA couldn't initialize GPIO %d, exiting",RELAY_PIN);
		return MRAA_ERROR_UNSPECIFIED;
	}
	
    sleep(1); //Delay after initialization 
	// set the pin as input
	if (mraa_gpio_dir(relay, MRAA_GPIO_OUT) != MRAA_SUCCESS) 
	{
		printf("Can't set digital pin %d as output, exiting",RELAY_PIN);
		return MRAA_ERROR_UNSPECIFIED;
	};

	// loop forever toggling the relay every 2 seconds
	while(1)
	{
		mraa_gpio_write(relay, 0);
		sleep(2);
		mraa_gpio_write(relay, 1);
		sleep(2);
	}

	return MRAA_SUCCESS;
}
