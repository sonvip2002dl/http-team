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
 * FILE NAME  :  runme_lab1.c      
 * DESCRIPTION:  This program reads button B1 and
 *               sends signal on UART to turn on LED.
 * *************************************************/
#include "mraa_beaglebone_pinmap.h"

mraa_gpio_context buttonPin;
mraa_uart_context uart;

int main(int argc, char** argv)
{
	int buttonState = 1;
	// The API frame below is device dependent
	char buf[] = {0x7E,0x00,0x10,0x17,0x01,0x00,0x13,0xA2,0x00,0x41,0x63,0x09,0x6F,0xFF,0xFE,0x02,0x50,0x30,0x05,0x92};
	char dev_string[] = "/dev/ttyS1";
	mraa_init();

	buttonPin = mraa_gpio_init(B1); 

	mraa_gpio_dir(buttonPin, MRAA_GPIO_IN);

	uart = mraa_uart_init_raw(dev_string);
	mraa_uart_set_baudrate(uart, 9600);
	mraa_uart_set_mode(uart, 8, MRAA_UART_PARITY_NONE , 1);
	mraa_uart_set_flowcontrol(uart, 0, 0);
	mraa_uart_set_timeout(uart, 0, 0, 0);

	for (;;) {
		buttonState = mraa_gpio_read(buttonPin);
		if (buttonState == 0) {
			// turn LED on:
			mraa_uart_write(uart, buf, sizeof(buf));
			printf ("ON\n");
			sleep(1);
		}
	}
	mraa_uart_stop(uart);
	mraa_deinit();
}
