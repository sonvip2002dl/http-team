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
 * FILE NAME  :  M1_L1_ButtonTest.c      
 * DESCRIPTION:  This code displays Buttons pressed
 *               on LCD "Buttons On xxxx"
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
	char buffer[30] = {0};
	mraa_gpio_context button1, button2, button3, button4;
	mraa_init();

	i2cp = mraa_i2c_init_raw (I2CP_BUS);
	mraa_i2c_frequency (i2cp, MRAA_I2C_STD);
	mraa_i2c_address(i2cp, LCD_ADDR);

	button1 = mraa_gpio_init(B1);
	button2 = mraa_gpio_init(B2);
	button3 = mraa_gpio_init(B3);
	button4 = mraa_gpio_init(B4);
	int last_val = 0;
	LCD_init();
	clear_LCD();

	while (1)
	{
		int button_val = 0;
		int B1_val = mraa_gpio_read(button1);
		int B2_val = mraa_gpio_read(button2);
		int B3_val = mraa_gpio_read(button3);
		int B4_val = mraa_gpio_read(button4);

		if(!B1_val) {

			button_val = 1;
		}
		if(!B2_val) {

			button_val = 10*button_val + 2;
		}
		if(!B3_val) {

			button_val = 10*button_val + 3;
		}
		if(!B4_val) {

			button_val = 10*button_val + 4;
		}
		if (button_val != last_val){
			clear_LCD();
		//	printf(" Last Val %d Button Val %d",last_val,button_val);
		}
		if (button_val == 0){
			sprintf (buffer, "Buttons on None   ");
		}
		else{
			sprintf (buffer, "Buttons on %d   ",button_val);
		}
		LCD_Print ((uint8_t*)buffer);
		last_val = button_val;
	}
	return 0;
}
