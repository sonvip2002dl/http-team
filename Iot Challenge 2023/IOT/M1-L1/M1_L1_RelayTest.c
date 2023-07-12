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
 * FILE NAME  :  M1_L1_RelayTest.c      
 * DESCRIPTION:  This code displays uses buttons 1
 *               and 2 to turn on and off the relay
 *               that should be connected to GP2
 *               This also indcates the relay status
 *               the LCD.
 ***************************************************/
#include "mraa_beaglebone_pinmap.h"

#define LCD_ADDR 		 0x3e
mraa_i2c_context i2cp;

void home_LCD (void)
{
	uint8_t buf[2] = {0x00,0x02};
	mraa_i2c_write(i2cp, buf, 2);  //Set to Home
}

void LCD_Print (uint8_t* str)
{
        uint8_t buf[80]={0};   // Set Buffer to all Null
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
	printf("initilizing/n");
	mraa_i2c_write(i2cp, init1, 2);
	mraa_i2c_write(i2cp, init2,8);  //Function Set
}

void clear_LCD (void)
{
	uint8_t buf[2] = {0x00,0x01};
	mraa_i2c_write(i2cp, buf, 2);  //Clear Display
}


int main(int argc, char** argv)
{
	mraa_result_t status = MRAA_SUCCESS;
	char buffer[20];
	mraa_gpio_context button1, button2, button3, button4,relay;
	mraa_init();

	i2cp = mraa_i2c_init_raw (I2CP_BUS);
	mraa_i2c_frequency (i2cp, MRAA_I2C_STD);
	mraa_i2c_address(i2cp, LCD_ADDR);

	button1 = mraa_gpio_init(B1);
	button2 = mraa_gpio_init(B2);
	button3 = mraa_gpio_init(B3);
	button4 = mraa_gpio_init(B4);
	relay = mraa_gpio_init(GP2);
	sleep(1);
	status=mraa_gpio_dir(relay, MRAA_GPIO_OUT);
        while (status != MRAA_SUCCESS) {
 		printf("Retrying Relay Pin Direction \n");
	        sleep(1);
  		status=mraa_gpio_dir(relay, MRAA_GPIO_OUT);
	}
	int relay_state = 1;  // Note 1 = off 0 = on
	int last_val = 0;
	LCD_init();
	clear_LCD();

	while (1)
	{
		status = mraa_gpio_write(relay,relay_state);
		if (status != MRAA_SUCCESS) {
			printf("GPIO Write Failed\n");
		}
		int B1_val = mraa_gpio_read(button1);
		int B2_val = mraa_gpio_read(button2);

		if(!B1_val) {

			relay_state = 0;
		}
		if(!B2_val) {

			relay_state =1;;
		}
		if (relay_state){
			sprintf (buffer, "Relay Off   ");
		}
		else{
			sprintf (buffer, "Relay On    ");
		}
		LCD_Print ((uint8_t*)buffer);
	}
	return 0;
}
