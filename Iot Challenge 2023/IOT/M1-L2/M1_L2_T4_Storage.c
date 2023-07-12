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
 * FILE NAME  :  M1_L2_T4_Storage.c      
 * DESCRIPTION:  This program logs and stores text
 *               unto a file
 ***************************************************/
 #include <stdio.h>
int main()
{
   FILE *fp;
   fp = fopen("/mnt/test.txt", "w+");
   fprintf(fp, "This is testing for U3800A...\n");
   fputs("This is testing for U3800A...\n", fp);
   fclose(fp);
   return 0;
}