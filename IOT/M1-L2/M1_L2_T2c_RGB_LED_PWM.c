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
 * FILE NAME  :  M1_L2_T2c_RGB_LED_PWM.c      
 * DESCRIPTION:  This code uses Pin 13 from BB P8 for Red 
 *               GP5_PWM for Green and GP6_PWM for Blue
 *               This sets the period to 30uS*255 = 7.6ms or 130Hz
 *               for all LED's.  This is designed for a 0 to 255 code
 *               to adjust the LED intensity much like most RGB systems
 *               This uses /sys/class/pwm writes to change the PWM pins.
 *               Note, this requires a clean shutdown at program exit.
 *               signal SIGINT provides method to add an exit function.
 * *************************************************/

#include "mraa_beaglebone_pinmap.h"
#include <signal.h>
int base = 30000; // 30us base increment
int period = 255; // base*period = 7.65 ms or 130 Hz cycle
int red_duty = 128;
int green_duty = 128;
int blue_duty = 128;

// set which pwm channel with in a pwmchip
int red_pwm_ch = 1;
int green_pwm_ch = 1;
int blue_pwm_ch = 0;

char pwm_per_red[] ="/sys/class/pwm/pwm-2:1/period";
char pwm_duty_red[] ="/sys/class/pwm/pwm-2:1/duty_cycle";
char pwm_enable_red[] ="/sys/class/pwm/pwm-2:1/enable";

char pwm_per_green[] ="/sys/class/pwm/pwm-0:1/period";
char pwm_duty_green[] ="/sys/class/pwm/pwm-0:1/duty_cycle";
char pwm_enable_green[] ="/sys/class/pwm/pwm-0:1/enable";

char pwm_per_blue[] ="/sys/class/pwm/pwm-0:0/period";
char pwm_duty_blue[] ="/sys/class/pwm/pwm-0:0/duty_cycle";
char pwm_enable_blue[] ="/sys/class/pwm/pwm-0:0/enable";

char pwm_chip_un0[] = "/sys/class/pwm/pwmchip0/unexport";
char pwm_chip_ex0[] = "/sys/class/pwm/pwmchip0/export";
char pwm_chip_un2[] = "/sys/class/pwm/pwmchip2/unexport";
char pwm_chip_ex2[] = "/sys/class/pwm/pwmchip2/export";

FILE *ppntr,*wpntr,*epntr;

int close_pwm()    // Disable all the PWM's otherwise they will remian active. 
		   // Initilizing active PWM pins will cause a segment fault. 
{
	epntr = fopen(pwm_enable_red,"w");
	if (epntr != NULL)
	{
		fprintf(epntr,"0");
		fclose(epntr);
	}
	epntr = fopen(pwm_enable_green,"w");
	if (epntr != NULL)
	{
		fprintf(epntr,"0");
		fclose(epntr);
	}
        epntr = fopen(pwm_enable_blue,"w");
	if (epntr != NULL)
	{
		fprintf(epntr,"0");
		fclose(epntr);
	}
	epntr = fopen(pwm_chip_un0,"w");
	if (epntr != NULL)
	{
		fprintf(epntr,"0");
		fclose(epntr);
	}
	epntr = fopen(pwm_chip_un0,"w");
	if (epntr != NULL)
	{
		fprintf(epntr,"1");
		fclose(epntr);
	}
       	epntr = fopen(pwm_chip_un2,"w");
	if (epntr != NULL)
	{
		fprintf(epntr,"1");
		fclose(epntr);
	}
}

int set_duty(int r_d, int g_d,int b_d)
{
	int pulse_w;
	wpntr = fopen(pwm_duty_red,"w");
	if (wpntr == NULL) 
	{
		printf("Can't set red pwm pin pulse width, exiting\n");
		close_pwm();
		return MRAA_ERROR_UNSPECIFIED;
	}
	pulse_w = (int) r_d*base;
	printf("Red Duty Cycle = %3.1f \%\n",r_d/2.55);
	fprintf(wpntr,"%d",pulse_w);
	fclose(wpntr);

	wpntr = fopen(pwm_duty_green,"w");
	if (wpntr == NULL) 
	{
		printf("Can't set green pwm pin pulse width, exiting\n");
		close_pwm();
		return MRAA_ERROR_UNSPECIFIED;
	}
	pulse_w = (int) g_d*base;
	printf("Green Duty Cycle = %3.1f \%\n",g_d/2.55);
	fprintf(wpntr,"%d",pulse_w);
	fclose(wpntr);

	wpntr = fopen(pwm_duty_blue,"w");
	if (wpntr == NULL) 
	{
		printf("Can't set blue pwm pin pulse width, exiting\n");
		close_pwm();
		return MRAA_ERROR_UNSPECIFIED;
	}
	pulse_w = (int) b_d*base;
	printf("Blue Duty Cycle = %3.1f \%\n",b_d/2.55);
	fprintf(wpntr,"%d",pulse_w);
	fclose(wpntr);

	return MRAA_SUCCESS;
}

int set_period(int per) // Note Only one PWM period per PWM pair needs to be set.
{
	int pulse_w;	
	ppntr = fopen(pwm_per_red,"w");
	if (ppntr == NULL) 
	{
		printf("Can't set red pwm pin period, exiting\n");
		close_pwm();
		return MRAA_ERROR_UNSPECIFIED;
	}
	fprintf(ppntr,"%d",per*base);
	fclose(ppntr);

	ppntr = fopen(pwm_per_green,"w");
	if (ppntr == NULL) 
	{
		printf("Can't set green/blue pwm pin period, exiting\n");
		close_pwm();
		return MRAA_ERROR_UNSPECIFIED;
	}
	fprintf(ppntr,"%d",per*base);
	fclose(ppntr);
	printf("Period set to %5.2f ms\n",(float)per*base/1000000);

	return MRAA_SUCCESS;

}

void exit_signal(int signum)
{
	printf("\nExiting PWM Program \n");
	close_pwm();
	exit(signum);
}

int main()
{
	mraa_result_t status = MRAA_SUCCESS;
	char buffer[20];
	mraa_init();
	int iten,red_dir,green_dir,blue_dir,dir_cntr;
	signal(SIGINT, exit_signal);
// Initialize Red LED PWM pwmchip2
	ppntr = fopen(pwm_per_red,"r");
	if (ppntr != NULL)
	{
		fclose(ppntr);
		printf("PWM already enabled\n");
	}
	else
	{
		epntr=fopen(pwm_chip_ex2,"w");
		printf("Enabling PWM\n");
		fprintf(epntr,"%d",red_pwm_ch);
		fclose(epntr);
	}
	

// Initialize Green LED PWM pwmchip0
	ppntr = fopen(pwm_per_green,"r");
	if (ppntr != NULL)
	{
		fclose(ppntr);
		printf("PWM already enabled\n");
	}
	else
	{
		epntr=fopen(pwm_chip_ex0,"w");
		printf("Enabling PWM\n");
		fprintf(epntr,"%d",green_pwm_ch);
		fclose(epntr);
	}
	
// Initialize Blue LED PWM pwmchip0
	ppntr = fopen(pwm_per_blue,"r");
	if (ppntr != NULL)
	{
		fclose(ppntr);
		printf("PWM already enabled\n");
	}
	else
	{
		epntr=fopen(pwm_chip_ex0,"w");
		printf("Enabling PWM\n");
		fprintf(epntr,"%d",blue_pwm_ch);
		fclose(epntr);
	}
	
	sleep(1); // ************  Note: A pause on enable is required!

// Now that the PWM chips have been enabled, set ther periods and duty cycles

	set_period(period);
	set_duty(red_duty,green_duty,blue_duty);

// Enable the outputs

	epntr = fopen(pwm_enable_red,"w");
	if (epntr == NULL) 
	{
		printf("Can't enable red pwm pin, exiting\n");
		close_pwm();
		return MRAA_ERROR_UNSPECIFIED;
	}
	fprintf(epntr,"%d",1);
	fclose(epntr);

	epntr = fopen(pwm_enable_green,"w");
	if (epntr == NULL) 
	{
		printf("Can't enable green pwm pin, exiting\n");
		close_pwm();
		return MRAA_ERROR_UNSPECIFIED;
	}
	fprintf(epntr,"%d",0);
	fclose(epntr);

	epntr = fopen(pwm_enable_blue,"w");
	if (epntr == NULL) 
	{
		printf("Can't enable blue pwm pin, exiting\n");
		close_pwm();
		return MRAA_ERROR_UNSPECIFIED;
	}
	fprintf(epntr,"%d",1);
	fclose(epntr);

	// loop forever looping pwm duty cycles.
	while(1)
	{
	    for (dir_cntr = 0;dir_cntr < 8;dir_cntr++)
	    {
		for (iten = 5;iten < 255;iten += 5)
		{
			red_dir = dir_cntr & 1;
			green_dir = (dir_cntr & 2)/2;
			blue_dir = (dir_cntr & 4)/4;
			red_duty = red_dir*iten + (1-red_dir)*(255 - iten);
			green_duty = green_dir*iten + (1-green_dir)*(255 - iten);
			blue_duty = blue_dir*iten + (1-blue_dir)*(255 - iten);
			set_duty(red_duty,green_duty,blue_duty);
			usleep(100000);
		}
  	    }
	
	}

	close_pwm();
	return MRAA_SUCCESS;
}
