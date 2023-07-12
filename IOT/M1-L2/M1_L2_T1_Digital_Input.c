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
 * FILE NAME  :  M1_L2_T1_Digital_Input.c      
 * DESCRIPTION:  This program outputs the status of
 *               button B3, pressed = 0 otherwise 1.
 * *************************************************/
 
 #include "mraa_beaglebone_pinmap.h"

int main()
{
	mraa_init();
	// create a GPIO object from MRAA using it
	mraa_gpio_context B3_pin;
	unsigned int value = 0;
	B3_pin = mraa_gpio_init(B3);
	if (B3_pin == NULL) 
	{
		fprintf(stderr, "MRAA couldn't initialize GPIO, exiting\n");
		return MRAA_ERROR_UNSPECIFIED;
	}
	

	// set the pin as input
	if (mraa_gpio_dir(B3_pin, MRAA_GPIO_IN) != MRAA_SUCCESS) 
	{
		fprintf(stderr, "Can't set digital pin as output, exiting\n");
		return MRAA_ERROR_UNSPECIFIED;
	};

	// to display the button value
	while(1) 
	{
		value = mraa_gpio_read(B3_pin);
		printf("B3 button value: %u\n", value);
		sleep(1);
	}

	return MRAA_SUCCESS;
}
