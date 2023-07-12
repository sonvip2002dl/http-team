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
 * FILE NAME  : M3_L2_T3_SPI.c      
 * DESCRIPTION: Displays the voltage measured on the 
 *              SPI pins
 *
 ***************************************************/
#include "mraa_beaglebone_pinmap.h"

#define LCD_ADDR 		 0x3e

mraa_i2c_context i2cp;
mraa_gpio_context MCP3208_DIN;
mraa_gpio_context MCP3208_DOUT;
mraa_gpio_context MCP3208_CLK;
mraa_gpio_context MCP3208_CS;

mraa_gpio_context button3;
mraa_gpio_context button4;

unsigned int channel;



void home_LCD (void)
{
	uint8_t buf[2] = {0x00,0x02};
	mraa_i2c_write(i2cp, buf, 2);  //Set to Home
}

void LCD_Print (char* str)
{
	uint8_t buf[80];
	uint8_t buf1[2]={0x00,0x80};

	int32_t i = 0, strl, j=0;

	buf[i] = 0x40;  //register for display
	strl = strlen((char*)str);
	for (j = 0; j < strl; j++)
	{
		i++;
		buf[i] = str[j];
	}
	mraa_i2c_write(i2cp, buf1, 2);
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

int main(int argc, char** argv)
{
	float voltage = 0;
	char buf[16];

	mraa_init();

	i2cp = mraa_i2c_init_raw (I2CP_BUS);
	mraa_i2c_frequency (i2cp, MRAA_I2C_STD);
	mraa_i2c_address(i2cp, LCD_ADDR);


	MCP3208_DIN = mraa_gpio_init (SPI_MOSI_PIN);
	MCP3208_DOUT = mraa_gpio_init (SPI_MISO_PIN);
	MCP3208_CLK = mraa_gpio_init (SPI_CLK_PIN);
	MCP3208_CS = mraa_gpio_init (SPI_CS0_PIN);


	mraa_gpio_dir(MCP3208_DIN, MRAA_GPIO_OUT_HIGH);
	mraa_gpio_dir(MCP3208_DOUT, MRAA_GPIO_IN);
	mraa_gpio_dir(MCP3208_CLK, MRAA_GPIO_OUT);
	mraa_gpio_dir(MCP3208_CS, MRAA_GPIO_OUT);

	channel = 0;
	LCD_init ();


	while (1)
	{
		clear_LCD();
		home_LCD();
		voltage = (float) GetMCP3208(channel)*3.3/4096 ;
		sprintf(buf, "Chan %u: %.4f V", channel, voltage);
		LCD_Print (buf);		
		usleep(500000);
	}
}
