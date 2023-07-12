/* ---------------------------------------------------------*/
/*            Copyright (c) 2020 Keysight Technologies      */
/*                                                                      */
/* PROPRIETARY RIGHTS of Keysight Technologies are involved in the  	*/
/* subject matter of this material. All manufacturing, reproduction,    */   
/* use, and sales rights pertaining to this subject matter are governed */
/* by the license agreement. The recipient of this software implicitly  */
/* accepts the terms of the license.                                    */
/* --------------------------------------------------------  */
/* FILE NAME  :  M3_L7_T4_ReceiveTemp.c                                 */
/* DESCRIPTION:  This code receives 16 bit signed integer    */
/*   ADC outputs corresponding to TMP36 temperatures from    */
/*   an XBee3 transmitted to a second XBee3 connected        */
/*   serially to the BeagleBone and displays the ADC         */
/*   reading, the voltage and the calulated temperature on   */
/*   the console                                             */
/*---------------------------------------------------------  */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdbool.h>
#include "mraa_beaglebone_pinmap.h"

#define LCD_ADDR		0x3e

mraa_i2c_context i2cp;
mraa_uart_context uart;
mraa_i2c_context i2cs;
mraa_gpio_context pin2;
mraa_gpio_context led5;

const unsigned char CHECK_SUM_VAL=0xFF;  // Target Check Sum value.
const bool DEBUG = false;                // Debug flag for extra printouts

double now(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_usec * 1e-6 + (double)tv.tv_sec;
}

bool find_start(mraa_uart_context uart, char find_char, double receive_timeout)
{
	char buf=0xff;      // Buffer for data
	unsigned char out;  // Output - needs to be unsigned for formatting to work right
	double start = now();
	int read_attempts=0; 
	int read_attempts_max=99; // try some reads
	do {
		if (mraa_uart_data_available(uart, 1.0 + receive_timeout * 1000.0)) 
		{
			mraa_uart_read(uart, &buf, 1);
			out = buf;
			if(DEBUG) printf(" 0x%02hhx ",out);
		}
		read_attempts++;
	} while ((read_attempts < read_attempts_max) && (buf != find_char));
	if (buf==find_char)
	     {
                     if(DEBUG) printf("Found Start\n");
	             return(true);
	     }
	     else 
	     {
		     printf("Start Byte Timeout - Retrying\n");
		     return(false);
	     }
}

bool read_data(mraa_uart_context uart, unsigned char *buffer, int byte_count, double receive_timeout)
{
	char buf=0xff;     // Buffer for data
	unsigned char out; // Output - needs to be unsigned for formatting to work right
	int i=0;
	int read_attempts=0; 
	int read_attempts_max=99; // try some reads
	do {
		if(mraa_uart_data_available(uart, 1.0 + receive_timeout * 1000.0)) 
		{
			mraa_uart_read(uart, &buf, 1);
			out = buf;
			if(DEBUG) printf(" 0x%02hhx ",out);
			buffer[i] = out;
			i++;
		}
		read_attempts++;
	} while ((read_attempts < read_attempts_max) && (i < byte_count));

	if(DEBUG) printf("\n i = %d read_attempts = %d\n", i, read_attempts);

	if (i == byte_count)   // Check if the received data count is correct
		{
        	if(DEBUG) printf("Got Data\n");
		return(true);
		}
	else 
		{
		printf("time out!\n");
		return(false);
		}
}

bool check_sum(unsigned char *buffer, int byte_count)
{
	unsigned char sum = 0;  // Note check sum is a byte ignore overflow
	for (int i = 0;i < byte_count;++i){
             sum += buffer[i];
	}
	if(DEBUG) printf("check sum = 0x%02hhx \n",sum);
	return( sum == CHECK_SUM_VAL);
}


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



int main(int argc, char** argv)
{
    double receive_timeout;
	// The API frame below is device dependent
	uart= NULL;
	mraa_init();
	char dev_string[] = "/dev/ttyS1";

	uart = mraa_uart_init_raw(dev_string);
	if(uart == NULL)
	{
		printf("Failed to setup UART\n");
		return 0;
	}

	mraa_uart_set_baudrate(uart, 9600);
	mraa_uart_set_mode(uart, 8, MRAA_UART_PARITY_NONE , 1);
	mraa_uart_set_flowcontrol(uart, 0, 0);
	mraa_uart_set_timeout(uart, 0, 0, 0);

	i2cp = mraa_i2c_init_raw (I2CP_BUS);
	mraa_i2c_frequency (i2cp, MRAA_I2C_STD);
	mraa_i2c_address(i2cp, LCD_ADDR);
	LCD_init();
	clear_LCD();
	home_LCD ();


	const double max_volt = 1.25;            // ADC voltage at max scale
	const int adc_scale = 1023;              // ADC Code at max scale
	const unsigned char start_byte = 0x7e;   // XBEE Starting frame byte.
	const unsigned char rx_adc_data = 0x92;  // ZBee Flag for ADC data being sent
	const int max_size = 50;                 // Maximum buffer size
	const int msb_off = 3;                   // MSB offset from end of buffer
	const int lsb_off = 2;                   // LSB offset from end of buffer
	const double tmp_off = 0.5;              // TMP36 offset = 500mV TMP35 offset = 0V
	const double long_timeout = 30.0;        // Time out in seconds before timing out on initial RX packet.
	const double short_timeout = 0.5;        // Timeout between data bytes.
	
	unsigned int byte_count=21;      // Number of bytes to receive
	int res;                         // ADC integer result 
	unsigned char buffer[max_size];  // buffer for results
	bool good;                       // Flag for good read
	double sensor;                   // Floating point ADC result
	
	while (1)
	{
        receive_timeout = long_timeout;
        if(find_start(uart,start_byte,receive_timeout))
	    {
          receive_timeout = short_timeout;
		  good = read_data(uart,buffer, 2, receive_timeout);
		  byte_count = buffer[0] *256 + buffer[1] +1; // need 1+ byte count to get check sum
		  if (DEBUG) printf(" Byte Count = %d\n",byte_count);
          if (good && byte_count <= max_size)
		  {
              good = read_data(uart, buffer, byte_count, receive_timeout);
		      if(good && check_sum(buffer,byte_count) && ( buffer[0] == rx_adc_data))
		      {
			     res = buffer[byte_count - msb_off]*256 + buffer[byte_count-lsb_off];
			     sensor = (double)max_volt*res/adc_scale;
			     sprintf((char*)buffer,"%6.2f c\n", (sensor - tmp_off)*100);
			     LCD_Print ((uint8_t*)buffer);

			     printf("Good Data! ADC Reading = %d  Sensor Voltage = %5.3fV Temp = %6.2f'C\n",res,sensor,(sensor - tmp_off)*100);
		      }
	       }     
        }
	}
	mraa_uart_stop(uart);
    mraa_deinit();
}

