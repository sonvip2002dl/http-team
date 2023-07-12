/* ------------------------------------------------------------------	*/
/*            Copyright (c) 2019 Keysight TechnologieS                  */
/*                                                                      */
/* PROPRIETARY RIGHTS of Keysight Technologies are involved in the  	*/
/* subject matter of this material. All manufacturing, reproduction,    */   
/* use, and sales rights pertaining to this subject matter are governed */
/* by the license agreement. The recipient of this software implicitly  */
/* accepts the terms of the license.                                    */
/* -------------------------------------------------------------------  */
/* FILE NAME  :  M3_L7_T4_sleepwake.c                                             */
/* DESCRIPTION:  This Lab code assigns three behaviors to buttons:      */
/*               B1 : Suspend power to BeagleBone                       */
/*               B2 : Start WiFi on BeagleBone                          */
/*               B3 : Stop WiFi on BeagleBone                           */
/*--------------------------------------------------------------------  */


#include "mraa_beaglebone_pinmap.h"

void LCD_Print (uint8_t* str);

#define LCD_ADDR 0x3E
#define LM75A_I2C_ADDR 0x48
#pragma GCC diagnostic ignored "-Wwrite-strings"

mraa_gpio_context reset;
mraa_gpio_context reset2; //LBS: add for testing
mraa_gpio_context led5;

mraa_i2c_context i2cp;
mraa_uart_context uart;
mraa_i2c_context i2cs;


void delay_time (unsigned int time_end)
{
	unsigned int index;
	for (index = 0; index < time_end; index++);
}

void LCD_Print (uint8_t* str)
{
        uint8_t buf[80];
		uint8_t buf1[2]={0x00,0x80};
		
        int32_t i = 0, strl, j=0;
 
        buf[i] = 0x40;  //register for display
		i++;
        strl = strlen((char*)str);
        for (j = 0; j < strl; j++)
        {
                buf[i] = str[j];
                i++;
        }
		mraa_i2c_write(i2cp, buf1, 2);
        mraa_i2c_write(i2cp, buf, i);
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

void home_LCD (void)
{
        uint8_t buf[2] = {0x00,0x02};
        mraa_i2c_write(i2cp, buf, 2);  //Set to Home
}

#define SENSOR_BUF 120

int main(int argc, char** argv)
{
	float T = 0;		// Store temperature data
	int32_t x = 0;
	int32_t i = 0;
	uint8_t buf [2] = {0, 0},buffer[6] ={0,0,0,0,0,0};
	char dataBuffer[SENSOR_BUF*6];
    char dev_string[] = "/dev/ttyS1";
	mraa_init();	 


	uart = mraa_uart_init_raw(dev_string);
	mraa_uart_set_baudrate(uart, 9600);
	mraa_uart_set_mode(uart, 8, MRAA_UART_PARITY_NONE , 1);
	mraa_uart_set_flowcontrol(uart, 0, 0);
	mraa_uart_set_timeout(uart, 0, 0, 0);
	if (uart == NULL) {
        fprintf(stderr, "UART failed to setup\n");
        return EXIT_FAILURE;
        }
	 
	
	i2cs = mraa_i2c_init_raw (I2CS_BUS);	
	mraa_i2c_frequency (i2cs, MRAA_I2C_STD);
	mraa_i2c_address(i2cs, LM75A_I2C_ADDR);
	
	i2cp = mraa_i2c_init_raw (I2CP_BUS);
	mraa_i2c_frequency (i2cp, MRAA_I2C_STD);
	mraa_i2c_address(i2cp, LCD_ADDR);
	
	LCD_init();

	LCD_Print ((uint8_t*)"Lab 2 Task 4");

	usleep(500000);
	usleep(500000);
	mraa_gpio_write(led5, 0);

	mraa_gpio_context button1, button2, button3;

	button1 = mraa_gpio_init(B1);
	button2 = mraa_gpio_init(B2);
	button3 = mraa_gpio_init(B3);

	while (1)
	{

		usleep(100000);
		int B1_val = mraa_gpio_read(button1);
		int B2_val = mraa_gpio_read(button2);
		int B3_val = mraa_gpio_read(button3);

		if(!B1_val)
		{
			home_LCD();
			clear_LCD();
			LCD_Print((uint8_t*)"PwrBtn Resume");
			system("systemctl suspend");
			sleep(5);
		}
		else if (!B2_val)
		{
			home_LCD();
			clear_LCD();
			LCD_Print((uint8_t*)"Start WiFi");
			system("systemctl start wpa_supplicant");
			sleep(5);
		}
		else if (!B3_val)
		{
			home_LCD();
			clear_LCD();
			LCD_Print((uint8_t*)"Stop WiFi");
			system("systemctl stop wpa_supplicant");
			sleep(5);
		}

		home_LCD();
		clear_LCD();
	LCD_Print((uint8_t*)"Push B1 B2 B3");
	}

}
