/* ------------------------------------------------------------------	*/
/*            Copyright (c) 2018 Keysight TechnologieS                  */
/*                                                                      */
/* PROPRIETARY RIGHTS of Keysight Technologies are involved in the  	*/
/* subject matter of this material. All manufacturing, reproduction,    */   
/* use, and sales rights pertaining to this subject matter are governed */
/* by the license agreement. The recipient of this software implicitly  */
/* accepts the terms of the license.                                    */
/* -------------------------------------------------------------------  */
/* FILE NAME  :  M2-L1-E7_Led.c                               			  	*/
/* DESCRIPTION:  This Lab code turns LED on when button is pressed      */
/*               										                */
/*--------------------------------------------------------------------  */

#include "mraa_beaglebone_pinmap.h"

#define LCD_ADDR		0x3e
#define LM75A_I2C_ADDR	0x48

mraa_i2c_context i2cp;
mraa_uart_context uart;
mraa_i2c_context i2cs;


void home_LCD (void)
{
        uint8_t buf[2] = {0x00,0x02};
        mraa_i2c_write(i2cp, buf, 2);  //Set to Home
}

void LCD_Print (uint8_t* str)
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


int main(void)
{
	
	char buffer[21]={0x7E,0x00,0x10,0x17,0x01,0x00,0x13,0xA2,0x00,0x41,0x98,0x42,0xD2,0xFF,0xFE,0x02,0x50,0x30,0x05,0xC1, 0xFE}; //API frame data
	char buffer2[21]={0x7E,0x00,0x10,0x17,0x01,0x00,0x13,0xA2,0x00,0x41,0x98,0x42,0xD2,0xFF,0xFE,0x02,0x50,0x30,0x04,0xC2, 0xFE}; //API frame data
	mraa_gpio_context button1, button2, button3, button4;
	mraa_init();
	char dev_string[] = "/dev/ttyS1";
	int buttonState = 1;

	button1 = mraa_gpio_init(B1);
	button2 = mraa_gpio_init(B2);
	button3 = mraa_gpio_init(B3);
	button4 = mraa_gpio_init(B4);
	
	mraa_gpio_dir(button1, MRAA_GPIO_IN);
	
	uart = mraa_uart_init_raw(dev_string);
	mraa_uart_set_baudrate(uart, 9600);
	mraa_uart_set_mode(uart, 8, MRAA_UART_PARITY_NONE , 1);
	mraa_uart_set_flowcontrol(uart, 0, 0);
	mraa_uart_set_timeout(uart, 0, 0, 0);
	if (uart == NULL) {
        fprintf(stderr, "UART failed to setup\n");
        return EXIT_FAILURE;
    }
	
	while (1)
	{		
		buttonState = mraa_gpio_read(button1);
  		if (buttonState == 0) {    			
			mraa_uart_write(uart, buffer, sizeof(buffer)); // turn LED on:
			printf ("ON\n");
			sleep(5);
		}
		else {
			mraa_uart_write(uart, buffer2, sizeof(buffer2)); // turn LED on:
			printf ("OFF\n");
			sleep(5);
		}
	}
	return 0;
}
