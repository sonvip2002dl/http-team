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
 * FILE NAME  :  M3_L3_T8_pressure.c
 * DESCRIPTION:  This program reads pressure values
 * 				 (in kPa) and calculated the altitude
 * 				 (in meter) with 1 second interval
 * 				 for 40 measurements. Then the program
 * 				 will write the output into
 * 				 a CSV file. 
 *
 **************************************************/

#include "mraa_beaglebone_pinmap.h"
#include <time.h>
#include <math.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BILLION 1E9

float alt, pressure, cTemp;
struct timespec start, stop, instant;
double time1, time2, time_elapse;
FILE *fp;

	int file;

	float offsetValue;

void initPressureSensor() 
{
	// Create I2C bus
	char* bus = (char*)"/dev/i2c-1";
	if((file = open(bus, O_RDWR)) < 0) 
	{
		printf("Failed to open the bus. \n");
		exit(1);
	}
	// Get I2C device, TSL2561 I2C address is 0x60(96)
	ioctl(file, I2C_SLAVE, 0x60);

	
}

void getPressure() {
	
	char reg[1] = {0x00};
	write(file, reg, 1);
	char data[6] = {0};
	char data2[6] = {0};
	
	char config[2] = {0};

	// Select control register(0x26)
	// Active mode, OSR = 128, barometer mode(0x39)
	config[0] = 0x26;
	config[1] = 0x39;
	write(file, config, 2);
	
	sleep(2);

	// Read 4 bytes of data from register(0x00)
	// status, pres msb1, pres msb, pres lsb
	reg[0] = 0x00;
	write(file, reg, 1);
	read(file, data, 4);


	// config[0] = 0x26;
	// config[1] = 0xB9;
	// write(file, config, 2);
	// sleep(1);
	// reg[0] = 0x00;
	// write(file, reg, 1);
	// read(file, data2, 6);
	// int tHeight = ((data2[1] * 65536) + (data2[2] * 256 + (data2[3] & 0xF0)) / 16);
	// alt = tHeight / 16.0;

	// Convert the data to 20-bits
	int pres = ((data[1] * 65536) + (data[2] * 256 + (data[3] & 0xF0))) / 16;
	pressure = (pres / 4.0) / 1000.0;

	alt = 44330.77 * (1 - pow(((pressure*1000)/101326),0.1902632));

}

void ReadPressure()
{
    fp = fopen("pressureResults.csv","w");

    char buf[20];
	int ret;
    int i;

	clock_gettime (CLOCK_REALTIME, &start);
    time1 = start.tv_sec + (float) start.tv_nsec / BILLION;
    for(i=0;i<40;i++) {
	// while(1){
        getPressure();
        fprintf(fp, "%5.4f, %5.2f\n", pressure, alt);
        printf("%d, %5.4f kPa, %5.2f m\n",(i+1), pressure, alt);
	}

    fclose(fp);
}


int main(void) {
	printf("40 measurements will be performed at 1 sec intervals.\n");
	initPressureSensor();
    ReadPressure();
}