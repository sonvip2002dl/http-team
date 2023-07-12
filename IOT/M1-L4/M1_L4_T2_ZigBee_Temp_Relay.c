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
 * FILE NAME  :  M1_L4_T2_ZigBee_Temp_Relay.c      
 * DESCRIPTION:  This code receives digital value 
 *				 from ZigBee, converts the value 
 *				 to temperature, and accordingly  
 *				 turns Relay ON/OFF.
 *
 * *************************************************/

#include "mraa_beaglebone_pinmap.h"

#define LCD_ADDR 		 0x3e

mraa_gpio_context Relay_pin;
mraa_i2c_context i2cp;


void home_LCD (void)
{
	uint8_t buf[2] = {0x00,0x02};
	mraa_i2c_write(i2cp, buf, 2);  //Set to Home
}

void LCD_Print (char* str)
{
	uint8_t buf[80];

	int32_t i = 0, strl, j=0;

	buf[i] = 0x40;  //register for display
	strl = strlen((char*)str);
	for (j = 0; j < strl; j++)
	{
		i++;
		buf[i] = str[j];
	}

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


void nextline_LCD (void)
{
	uint8_t buf[2] = {0x00,0xC0};

	mraa_i2c_write(i2cp, buf, 2);  //2nd line of LCD Display
}


unsigned int threshold=25;

int main(int argc, char** argv)
{
	char buffer [1024], buf_Xbee[1024];
	unsigned int ADC = 0, timeout = 2000;
	double Temp = 0, V36=0;
	char dev_string[] = "/dev/ttyS1";
	
	mraa_init();

	i2cp = mraa_i2c_init_raw (I2CP_BUS);
	mraa_i2c_frequency (i2cp, MRAA_I2C_STD);
	mraa_i2c_address(i2cp, LCD_ADDR);

	Relay_pin = mraa_gpio_init(RELAY_PIN);
	mraa_gpio_dir(Relay_pin, MRAA_GPIO_OUT);	

	mraa_uart_context uart;	
	mraa_boolean_t isDataReady;
	uart = mraa_uart_init_raw(dev_string);
	mraa_uart_set_baudrate(uart, 9600);
	mraa_uart_set_mode(uart, 8, MRAA_UART_PARITY_NONE , 1);
	mraa_uart_set_flowcontrol(uart, 0, 0);
	mraa_uart_set_timeout(uart, 0, 0, 0);
	if (uart == NULL)
	{
		printf("UART failed to setup\n");
		return 0;
	}

	LCD_init();
	clear_LCD();
	home_LCD();
	LCD_Print ((char*)("Keysight U3810A"));
	sleep(1);

	mraa_gpio_write(Relay_pin, 0);

	while (1)
	{
		isDataReady = mraa_uart_data_available(uart, timeout);
		if (isDataReady == 1)
		{
			mraa_uart_read(uart, buf_Xbee, sizeof(buffer));
			
			ADC = (buf_Xbee[21] & 0x3)<<8 | buf_Xbee[22];
			V36 = (ADC * 1250 )/1023;
			Temp = (V36- 500)/10;
			
			printf("Sensor:%3.1f'C, Threshold:%u'C\n", Temp, threshold);

			sprintf(buffer, "Sensor:%3.1f %u ", Temp, threshold);

			clear_LCD();
			home_LCD();
			LCD_Print(buffer);

			nextline_LCD();

			if(Temp>threshold)
			{
				sprintf(buffer, "Relay On ");
				LCD_Print(buffer);
				mraa_gpio_write(Relay_pin, 0);
				sleep(1);
			}
			else
			{
				sprintf(buffer, "Relay Off ");
				LCD_Print(buffer);
				mraa_gpio_write(Relay_pin, 1);
				sleep(1);
			}
		}
		else
		{
			printf("Error: Timeout. UART data not retrieved after %u ms\n", timeout);
		}
	}
	
	return 0;
}


