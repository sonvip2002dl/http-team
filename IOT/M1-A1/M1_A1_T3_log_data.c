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
 * FILE NAME  :  M1_A1_T3_log_data.c      
 * DESCRIPTION:  This code records data from 4
 *				 sensors into text file log.txt
 *
 * *************************************************/
 
#include <stdio.h>

int main(int argc, char** argv)
{	
	FILE *fp;
	fp = fopen("/home/debian/LabCode/M1-A1/log.txt", "a+");
	fprintf(fp, "%s %s %s %s\n", argv[1], argv[2], argv[3], argv[4]);
	fclose(fp);
}