/*************************************************
 *         © Keysight Technologies 2019 
 *
 *  PROPRIETARY RIGHTS of Keysight Technologies are 
 *  involved in the subject matter of this software. 
 *  All manufacturing, reproduction, use, and sales 
 *  rights pertaining to this software are governed 
 *  by the license agreement. The recipient of this 
 *  code implicitly accepts the terms of the license. 
 *
 ***************************************************
 *
 * FILE NAME  :  M1_L1_TMP36.c      
 * DESCRIPTION:  This code prints the output voltage
 *               of TMP36 as read by ADC.
 *
 ***************************************************/
#include "mraa_beaglebone_pinmap.h"

mraa_gpio_context MCP3208_DIN;
mraa_gpio_context MCP3208_DOUT;
mraa_gpio_context MCP3208_CLK;
mraa_gpio_context MCP3208_CS;

int GetMCP3208 (int Channel);

int main()
{
	int channel = 7;

	MCP3208_DIN = mraa_gpio_init (SPI_MOSI_PIN);
	MCP3208_DOUT = mraa_gpio_init (SPI_MISO_PIN);
	MCP3208_CLK = mraa_gpio_init (SPI_CLK_PIN);
	MCP3208_CS = mraa_gpio_init (SPI_CS0_PIN);

	mraa_gpio_dir(MCP3208_DIN, MRAA_GPIO_OUT_HIGH);
	mraa_gpio_dir(MCP3208_DOUT, MRAA_GPIO_IN);
	mraa_gpio_dir(MCP3208_CLK, MRAA_GPIO_OUT);
	mraa_gpio_dir(MCP3208_CS, MRAA_GPIO_OUT);

	printf("ADC Value at Channel %d: %d\n", channel, GetMCP3208 (channel));


	return MRAA_SUCCESS;
}

int GetMCP3208 (int Channel)
{
	int i;
	int val;

	mraa_gpio_write (MCP3208_DIN, 0);
	mraa_gpio_write (MCP3208_CLK, 0);
	mraa_gpio_write (MCP3208_CS, 0);

	Channel = Channel | 0x18;
	for (i = 0; i < 5; i ++)
	{
		if (Channel & 0x10)
		{
			mraa_gpio_write (MCP3208_DIN, 1);
		}
		else
		{
			mraa_gpio_write (MCP3208_DIN, 0);
		}
		Channel <<= 1;

		mraa_gpio_write (MCP3208_CLK, 0);
		mraa_gpio_write (MCP3208_CLK, 1);
	}

	mraa_gpio_write (MCP3208_CLK, 0);
	mraa_gpio_write (MCP3208_CLK, 1);

	mraa_gpio_write (MCP3208_CLK, 0);
	mraa_gpio_write (MCP3208_CLK, 1);

	val = 0;
	for (i = 0; i < 12; i ++)
	{
		mraa_gpio_write (MCP3208_CLK, 0);
		mraa_gpio_write (MCP3208_CLK, 1);

		val = (val << 1) | ((int) mraa_gpio_read (MCP3208_DOUT));	
	}

	mraa_gpio_write (MCP3208_CS, 1);
	mraa_gpio_write (MCP3208_DIN, 0);
	mraa_gpio_write (MCP3208_CLK, 0);

	return val;
}
