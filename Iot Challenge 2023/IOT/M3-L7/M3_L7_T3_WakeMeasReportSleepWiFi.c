/* ------------------------------------------------------------------	*/
/*            Copyright (c) 2020 Keysight Technologies                  */
/*                                                                      */
/* PROPRIETARY RIGHTS of Keysight Technologies are involved in the  	*/
/* subject matter of this material. All manufacturing, reproduction,    */   
/* use, and sales rights pertaining to this subject matter are governed */
/* by the license agreement. The recipient of this software implicitly  */
/* accepts the terms of the license.                                    */
/* -------------------------------------------------------------------  */
/* FILE NAME  :  M3_L6_T3_WakeMeasReportSleepWiFi.c                     */
/* DESCRIPTION: To Run this lab code the LM75A sensor needs to be       */
/* Connected. This labcode calls M3_T3_Temp_Post_MQTT to read the       */
/*temperature sensor and post the data to an MQTT broker via wifi       */ 
/*Set the MQTT Broker ip address in the   MQTT_Broker_ip definition     */
/*								 	*/
/* Must run as sudo!!!!!                                                 */
/*									 */
/* Test types                                                            */
/* 0. Idle0TX         XBee and BeagleBone always powered                 */
/* 1. Idle30TX        XBee and BeagleBone always powered 30 second wait  */
/* 2. Sleep30TX       Get/transmit sensor, XBee+BB sleep 30 seconds      */
/* 3. Sleep45TX       Get/transmit sensor, XBee+BB sleep 30 seconds      */
/* 4. Sleep60TX       Get/transmit sensor, XBee+BB sleep 30 seconds      */
/* 5. SleepNoTX       Sleep XB, Read Sensor, Wake/Transmit XB, Sleep XB  */
/* 6. Sleep30NoTX     Sleep XB, Read/Transmit data, sleep all 30 sec     */
/* 7. Sleep45NoTX     Sleep XB, Read/Transmit Data, Sleep all 45 sec     */
/* 8. Sleep60NoTX     Sleep XB, Read/Transmit Data, Sleep all 60 sec     */
/*                                                                       */
/*--------------------------------------------------------------------   */

#include "mraa_beaglebone_pinmap.h"

#define LCD_ADDR	0x3e
#define LM75A_I2C_ADDR	0x48

/* Test types defined   */
#define Idle0TX		0
#define Idle30TX	1
#define Sleep30TX	2
#define Sleep45TX	3
#define Sleep60TX	4
#define SleepNoTX	5
#define Sleep30NoTX	6
#define Sleep45NoTX	7
#define Sleep60NoTX	8

#define TRUE	1
#define FALSE	0
#define MQTT_Broker_ip "192.168.10.123"

mraa_i2c_context i2cp;
mraa_uart_context uart;
mraa_i2c_context i2cs;
mraa_gpio_context pin2;
mraa_gpio_context led5;

void delay_time (unsigned int time_end)
{
	unsigned int index;
	for (index = 0; index < time_end; index++);
}

#define SENSOR_BUF 100
int main(void)
{
	float T = 0;		// Store temperature data
	int32_t x = 0;
	uint8_t buf [2] = {0, 0},buffer[7] ={0,0,0,0,0,0,0};
	uint8_t tempval = 4;
	int i=0;
	int testType=Idle30TX;
	int Tx = FALSE;
	int Sleep = FALSE;
	char dataBuffer[SENSOR_BUF*6];
        char dev_string[] = "/dev/ttyS1";
	char ip_string[] = MQTT_Broker_ip;
	mraa_init();
	char command_buf[80];  // buffer for system command	 
	unsigned int count=0, read_count=0;
	usleep(500000);

	mraa_gpio_write(led5, 0);
	
	mraa_gpio_write(pin2, 0);

	
	
	while (1)
	{
		
		usleep(100000);	
		sprintf((char*)command_buf,"python ./M3_L7_Temp_Post_MQTT.py  %s 1",ip_string ); //print to string for system command
		system(command_buf);
		system( "rtcwake -d /dev/rtc0 -m standby -s 30");	//sleep BeagleBone 30sec
		usleep(3000000);	// Wait 3 seconds for WiFi connection to be established. Time to be adjusted empirically
		
		
	}   // end while

	return 0;
}
