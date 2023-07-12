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
 * FILE NAME  :  M1_L2_E2_PWM.c      
 * DESCRIPTION:  Manipulates the PWM signal 
 * *************************************************/

#include "mraa_beaglebone_pinmap.h"
#include <signal.h>
int period = 20000000; // 20 ms or 50 Hz cycle
float duty_cycle = 0.2; // 20 % duty cycle

char pwm_per[] ="/sys/class/pwm/pwm-0:1/period";
char pwm_pulse_w[] ="/sys/class/pwm/pwm-0:1/duty_cycle";
char pwm_enable[] ="/sys/class/pwm/pwm-0:1/enable";
char pwm_chip_un[] = "/sys/class/pwm/pwmchip0/unexport";
char pwm_chip_ex[] = "/sys/class/pwm/pwmchip0/export";
FILE *ppntr,*wpntr,*epntr;


int close_pwm()
{
	epntr = fopen(pwm_enable,"w");
	if (epntr != NULL)
	{
		fprintf(epntr,"0");
		fclose(epntr);
	}
	epntr = fopen(pwm_chip_un,"w");
	if (epntr != NULL)
	{
		fprintf(epntr,"0");
		fclose(epntr);
	}
}
int set_duty(float duty)
{
	int pulse_w;
	wpntr = fopen(pwm_pulse_w,"w");
	if (wpntr == NULL) 
	{
		printf("Can't set pwm pin pulse width, exiting\n");
		close_pwm();
		return MRAA_ERROR_UNSPECIFIED;
	}
	pulse_w = (int) period*duty;
	printf("Duty Cycle = %3.1f \%\n",duty*100);
	fprintf(wpntr,"%d",pulse_w);
	fclose(wpntr);
	return MRAA_SUCCESS;
}
int set_period(int per)
{
	int pulse_w;	
	ppntr = fopen(pwm_per,"w");
	if (ppntr == NULL) 
	{
		printf("Can't set pwm pin period, exiting\n");
		close_pwm();
		return MRAA_ERROR_UNSPECIFIED;
	}
	fprintf(ppntr,"%d",per);
	fclose(ppntr);
	
	wpntr = fopen(pwm_pulse_w,"w");
	if (wpntr == NULL) 
	{
		printf("Can't set pwm pin pulse width, exiting\n");
		close_pwm();
		return MRAA_ERROR_UNSPECIFIED;
	}
	pulse_w = (int) per*duty_cycle;
	printf("Period = %d msec\n", per/1000000);
	fprintf(wpntr,"%d",pulse_w);
	fclose(wpntr);
	return MRAA_SUCCESS;
}
void exit_signal(int signum)
{
	printf("\nExiting PWM Program \n");
	close_pwm();
	exit(signum);
}
int setup_pwm()
{
	ppntr = fopen(pwm_per,"r");
	if (ppntr != NULL)
	{
		fclose(ppntr);
		printf("PWM already enabled\n");
	}
	else
	{
		epntr=fopen(pwm_chip_ex,"w");
		printf("Enabling PWM\n");
		fprintf(epntr,"1");
		fclose(epntr);
	}
	// set the PWM period in uS
	ppntr = fopen(pwm_per,"w");
	if (ppntr == NULL) 
	{
		printf("Can't set pwm pin period, exiting\n");
		close_pwm();
		return MRAA_ERROR_UNSPECIFIED;
	}
	fprintf(ppntr,"%d",period);
	printf("\n done \n");
	fclose(ppntr);

	wpntr = fopen(pwm_pulse_w,"w");
	if (wpntr == NULL) 
	{
		printf("Can't set pwm pin pulse width, exiting\n");
		close_pwm();
		return MRAA_ERROR_UNSPECIFIED;
	}
	fprintf(wpntr,"%d",(int)period*0.1);
	fclose(wpntr);

	epntr = fopen(pwm_enable,"w");
	if (epntr == NULL) 
	{
		printf("Can't enable pwm pin, exiting\n");
		close_pwm();
		return MRAA_ERROR_UNSPECIFIED;
	}
	fprintf(epntr,"%d",1);
	fclose(epntr);
}

int main()
{
	int status=0;
	float dty_cycle;
	int per;
	signal(SIGINT, exit_signal);
	setup_pwm();
	
	// loop forever ramping the period from 10 ms to 100 ms
	while(1)
	{
		for (per = 10000000;per < 100000000;per += 10000000)
		{
			set_period(per);
			sleep(5);
		}
	}

	close_pwm();
	return MRAA_SUCCESS;
}
