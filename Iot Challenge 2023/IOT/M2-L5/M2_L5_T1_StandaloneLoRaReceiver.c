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
 * FILE NAME  :  StandaloneLoRaReceiver.c      
 * DESCRIPTION:  Receives data repeatedly using LoRa
 *
 * *************************************************/
#include "mraa_beaglebone_pinmap.h"
#include <time.h>

#define MYT +8

mraa_uart_context uart;

void delay_time (unsigned int time_end){
	unsigned int index;
	for (index = 0; index < time_end; index++);
}

void send_cmd (char* cmd){
	char* temp = cmd;
	for (; *temp != '\0'; temp ++)
	{
		mraa_uart_write(uart, temp, 1);
	}
	mraa_uart_write(uart, "\r\n", 2);
	delay_time (300);
	mraa_uart_flush(uart);
	char rep[20] = "";
	mraa_uart_read(uart,rep,20);
	//printf ("%s: %s\n",cmd,rep);
}

int main(void)
{
	uart= NULL;
	mraa_init();
	char dev_string[] = "/dev/ttyS2";

	uart = mraa_uart_init_raw(dev_string);
	if(uart == NULL)
	{
		printf("Failed to setup UART\n");
		return 0;
	}

	mraa_uart_set_baudrate(uart, 57600);
	mraa_uart_set_mode(uart, 8, MRAA_UART_PARITY_NONE, 1);
	mraa_uart_set_flowcontrol(uart, 0, 0);
	mraa_uart_set_timeout(uart, 0, 0, 0);

	send_cmd((char*)"radio cw off");
	sleep(2);
	send_cmd((char*)"radio set freq 433050000");
	send_cmd((char*)"radio set wdt 0");
	printf ("waiting for data...\n\n");
	while (1) 
	{
		send_cmd((char*)"mac pause");
		send_cmd((char*)"radio rx 0");
		sleep (5);
		char rx[265] = "";
		mraa_uart_read(uart,rx,265);
		printf ("%s\n",rx);
	}
	
	mraa_uart_stop(uart);
	mraa_deinit();
	return 0;
}
