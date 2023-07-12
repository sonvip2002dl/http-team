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
 * FILE NAME  :  M1_L2_E4a_Storage_log.c      
 * DESCRIPTION:  This code logs current date and time 
 *               and saves it unto a text file
 ***************************************************/
#include <stdio.h>
#include <time.h>
int main()
{
   FILE *fp;
   time_t t = time(NULL);
   struct tm tm = *localtime(&t);
   
   fp = fopen("/home/debian/log.txt", "w+");
   fprintf(fp, "Log report: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
   fclose(fp);
   return 0;
}