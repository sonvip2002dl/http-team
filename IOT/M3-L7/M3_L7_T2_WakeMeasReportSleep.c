/* ------------------------------------------------------------------	 */
/*            Copyright (c) 2020 Keysight TechnologieS                   */
/*                                                                       */
/* PROPRIETARY RIGHTS of Keysight Technologies are involved in the  	 */
/* subject matter of this material. All manufacturing, reproduction,     */   
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement. The recipient of this software implicitly   */
/* accepts the terms of the license.                                     */
/* -------------------------------------------------------------------   */
/* FILE NAME  :  M3_L7_T2_WakeMeasReportSleep.c                          */
/* DESCRIPTION:  This Lab code reads 100 temperatures from the LM75,     */
/*               save them to a buffer, displays one of them on the LCD, */
/*               and transmits the buffer to the Zigbee Gateway, with    */
/*               various power scenarios for analysis.                   */
/* Connect PC to U3810A J15 using serial com port, 115200 baud.          */
/* Must run as sudo:    sudo ./M3_L7_T2_WakeMeasReportSleep              */
/* Test types                                                            */
/* 0. Idle0        Get/transmit sensor, rpt (XBee and BB always powered) */
/* 1. Idle30       Get/transmit sensor, 30 sec wait, rpt                 */
/* 2. SleepBB30    Get/transmit sensor, BeagleB sleep 30 seconds, rpt    */
/* 3. SleepBB45    Get/transmit sensor, BeagleB sleep 45 seconds, rpt    */
/* 4. SleepBB60    Get/transmit sensor, BeagleB sleep 60 seconds, rpt    */
/* 5. SleepBBXB30  Get/transmit sensor, XB+BeagleB sleep 30 seconds, rpt */
/* 6. SleepBBXB45  Get/transmit sensor, XB+BeagleB sleep 45 seconds, rpt */
/* 7. SleepBBXB60  Get/transmit sensor, XB+BeagleB sleep 60 seconds, rpt */
/*                                                                       */
/* main outline:  Wake XB                                                */
/*                Get/transmit sensor/Display                            */
/*                Idle 30 if case 1                                      */
/*                Sleep XB if case = 5, 6 or 7                           */
/*                Sleep BB for a time, then wake BB if case = 2 - 7      */
/*                repeat                                                 */
/*                                                                       */
/*--------------------------------------------------------------------   */

#include "mraa_beaglebone_pinmap.h"
#include <stdio.h>

#define LCD_ADDR	0x3e
#define LM75A_I2C_ADDR	0x48

#define TRUE	1
#define FALSE	0

mraa_i2c_context i2cp;
mraa_uart_context uart;
mraa_i2c_context i2cs;

void delay_time (unsigned int time_end)
{
	unsigned int index;
	for (index = 0; index < time_end; index++);
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

#define SENSOR_BUF 100

int main(void)
{
int TestType;
printf ("Enter Type (0 Idle0, 1 Idle30, 2 SleepBB30, 3 SleepBB45, 4 SleepBB60, 5 SleepBBXB30, 6 SleepBBXB45, 7 SleepBBXB60): ");
scanf("%d",&TestType);  // read in number of test type from stdio
if ((TestType>8) || (TestType<0)) TestType = 0;
printf ("%d", TestType); 
switch (TestType){
	case 0:	
		printf (" Idle0\n");
		break;
	case 1: 
		printf (" Idle30\n");
		break;
	case 2:	
		printf (" SleepBB30\n");
		break;
	case 3: 
		printf(" SleepBB45\n");
		break; 
	case 4: 
		printf (" SleepBB60\n");
		break;
	case 5: 
		printf (" SleepBBXB30\n");
		break;
	case 6: 
		printf (" SleepBBXB45\n");
		break;
	case 7: 
		printf (" SleepBBXB60\n");
		break;
}

mraa_result_t status = MRAA_SUCCESS;
mraa_gpio_context sleep_rq;
mraa_init();
sleep_rq = mraa_gpio_init(GP2);	 
sleep(1);
status = mraa_gpio_dir(sleep_rq, MRAA_GPIO_OUT);
while (status != MRAA_SUCCESS) {
 	printf("Retrying SLEEP_RQ Pin Direction \n");
        sleep(1);
  	status=mraa_gpio_dir(sleep_rq, MRAA_GPIO_OUT);
}
while (1)  // main loop
{

/* Wake XBee3                                                           */
/* Assure that the XBee3 is awake by writing its SLEEP_RQ pin low.      */
/*                                                                      */
	status = mraa_gpio_write(sleep_rq,0); // wake XBee3
	if (status != MRAA_SUCCESS) {
		printf("SLEEP_RQ GPIO Write Failed\n");
	}

/* Get/transmit sensor/Display                                          */                                     
/*                                                                      */
	float T = 0;		
	int32_t x = 0;
	uint8_t buf [2] = {0, 0},buffer[7] ={0,0,0,0,0,0,0};
	uint8_t tempval = 4;
	int i=0;
	
	char dataBuffer[SENSOR_BUF*6];
        char dev_string[] = "/dev/ttyS1";

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
	clear_LCD();
	home_LCD ();
	LCD_Print ((uint8_t *)"Lab 7 Task 2");
	
	unsigned int count=0, read_count=0;
	usleep(500000);
	while (read_count<SENSOR_BUF)  //  measurement loop
	{
		usleep(100000);	
		mraa_i2c_read_bytes_data(i2cs,0x0, buf, 2);
		tempval = mraa_i2c_read_byte_data(i2cs, 0x1);
		printf("read_count:  %d\n", read_count);
		x = (buf [0] << 8) | buf [1];
		x = (x & 0x7FFF) >> 7;
		T = x / 2.0;
		if (buf [0] & 0x80)	// if <0
		{
			sprintf((char*)buffer,"-%3.1f\n", T);				
		}
		else
		{
			sprintf((char*)buffer,"+%3.1f\n", T);		
		}
		
		if(count==10)	// Display data value every 1s
		{
			count=0;
			clear_LCD();
			home_LCD ();
			LCD_Print(buffer);
		}

		for(i=0; i<6; i++)
			dataBuffer[i + read_count*6] = buffer[i];

		// Transmit data when buffer is full

		if(read_count==SENSOR_BUF-1)
		{
			clear_LCD();
			home_LCD();
			LCD_Print((uint8_t *)"Transmit Data");
			sleep(1);	// wait for the XBee to fully wake up
			for(i=0; i<SENSOR_BUF; i++)
			{
				mraa_uart_write(uart, dataBuffer+i*6, 6);
				usleep(100000);
			}
		}
		count++;
		read_count++;
	}  // end while(1) measurement loop
	read_count = 0;  // reset counter

/*    Idle 30 if case 1 Idle30                                          */
/*                                                                      */
	if (TestType == 1) {
			clear_LCD();
			home_LCD();
			LCD_Print((uint8_t *)"Sleep 30 sec");
			sleep(30);
	}

/*    Sleep XB if case = 5, 6 or 7                                      */
/*                                                                      */
	if((TestType == 5)||(TestType == 6)||(TestType == 7))
	{
		printf("Sleep XB\n");
		status = mraa_gpio_write(sleep_rq,1); // sleep XBee3
		if (status != MRAA_SUCCESS) {
			printf("SLEEP_RQ GPIO Write Failed\n");
		}
	}
		
/*    Sleep BB for a time, then wake BB if case = 2 - 7  (seconds)       */
/*                                                                       */
	if((TestType == 2)||(TestType == 5))
	{
		system( "rtcwake -d /dev/rtc0 -m standby -s 30");
	}
	if((TestType == 3)||(TestType == 6))
	{
		system( "rtcwake -d /dev/rtc0 -m standby -s 45");
	}
	if((TestType == 4)||(TestType == 7))
	{
		system( "rtcwake -d /dev/rtc0 -m standby -s 60");
	}

}
return 0;
}