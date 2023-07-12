/* ------------------------------------------------------------------	*/
/*            Copyright (c) 2018 Keysight TechnologieS                  */
/*                                                                      */
/* PROPRIETARY RIGHTS of Keysight Technologies are involved in the  	*/
/* subject matter of this material. All manufacturing, reproduction,    */   
/* use, and sales rights pertaining to this subject matter are governed */
/* by the license agreement. The recipient of this software implicitly  */
/* accepts the terms of the license.                                    */
/* -------------------------------------------------------------------  */
/* FILE NAME  :  M3_L5_T2_static.c                                 */
/* DESCRIPTION:  This Lab code reads temperature from LM75, displays    */
/*               it on LCD and sends to the Zigbee Gateway.              */
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


int main(void)
{
	
	float T = 0;		// Store temperature data
	int32_t x = 0;
	uint8_t buf [2] = {0, 0},buffer[7] ={0,0,0,0,0,0,0};
	uint8_t tempval = 4;
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
	
	
	while (1)
	{
		
		mraa_i2c_read_bytes_data(i2cs,0x0, buf, 2);
		tempval = mraa_i2c_read_byte_data(i2cs, 0x1);

		
		x = (buf [0] << 8) | buf [1];
		x = (x & 0x7FFF) >> 7;
		T = x / 2.0;
		if (buf [0] & 0x80)
		{
			sprintf((char*)buffer,"-%3.1f\n", T);
						
		}
		else
		{
			sprintf((char*)buffer,"+%3.1f\n", T);
						
		}
		mraa_uart_write(uart, (char*)buffer, 6);
		
		clear_LCD();
		home_LCD ();
			
		LCD_Print(buffer);
		sleep(1); // Display data value every 1s
		
	}
	return 0;
}
