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
 * FILE NAME  :  pwm_py.c      
 * DESCRIPTION:  This program receives and configures 
 *               the duty cycle of the PWM and 
 *				 sends out the PWM signal
 * *************************************************/

#include "mraa_beaglebone_pinmap.h"
#include <signal.h>
int period = 200000000; // 20 ms or 50 Hz cycle
float duty_cycle = 0.5; // 20 % duty cycle

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
int pwm_set_duty(float duty)
{
	int pulse_w;
	duty_cycle = duty;
	wpntr = fopen(pwm_pulse_w,"w");
	if (wpntr == NULL) 
	{
		printf("Can't set pwm pin pulse width, exiting\n");
		close_pwm();
		return MRAA_ERROR_UNSPECIFIED;
	}
	pulse_w = (int) period*duty;
	fprintf(wpntr,"%d",pulse_w);
	fclose(wpntr);
	return MRAA_SUCCESS;
}
int pwm_set_period(int per)
{
	int pulse_w;	
	period = per;
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
	fprintf(wpntr,"%d",pulse_w);
	fclose(wpntr);
	return MRAA_SUCCESS;
}
void pwm_exit_signal(int signum)
{
	printf("\nExiting PWM Program \n");
	close_pwm();
	exit(signum);
}

int main(int argc, char *argv[])
{
	float dty_cycle;
	int per;
	signal(SIGINT, pwm_exit_signal);
	ppntr = fopen(pwm_per,"r");
	if (ppntr != NULL)
	{
		fclose(ppntr);
	}
	else
	{
		epntr=fopen(pwm_chip_ex,"w");
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
	
	pwm_set_duty(atof(argv[1]));

	return MRAA_SUCCESS;
}
