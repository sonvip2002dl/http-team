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
 * FILE NAME  :  M1_L2_E1_buttons.c      
 * DESCRIPTION:  read the input from all the buttons 
 *               B1-B4 and return the corresponding 
 *               message when the buttons are pressed.
 * *************************************************/

#include "mraa_beaglebone_pinmap.h"

mraa_gpio_context pb1;
mraa_gpio_context pb2;
mraa_gpio_context pb3;
mraa_gpio_context pb4;

int main(int argc, char** argv)
{
	int x;
	mraa_init();

	pb1 = mraa_gpio_init(B1);
	pb2 = mraa_gpio_init(B2);
	pb3 = mraa_gpio_init(B3);
	pb4 = mraa_gpio_init(B4);

	mraa_gpio_dir(pb1, MRAA_GPIO_IN);
	mraa_gpio_dir(pb2, MRAA_GPIO_IN);
	mraa_gpio_dir(pb3, MRAA_GPIO_IN);
	mraa_gpio_dir(pb4, MRAA_GPIO_IN);

	while (1)
	{	
		printf("Press button B1 ~ B4:\n");
		while (1)

		{
			if (mraa_gpio_read (pb1) == 0)

			{
				printf("Button 1 pressed.\n");
				break;
			}

			if (mraa_gpio_read (pb2) == 0)
			{
				printf("Button 2 pressed.\n");
				break;
			}

			if (mraa_gpio_read (pb3) == 0)
			{
				printf("Button 3 pressed.\n");
				break;
			}

			if (mraa_gpio_read (pb4) == 0)
			{
				printf("Button 4 pressed.\n");
				break;
			}
		}

		sleep (1);
	}
}
