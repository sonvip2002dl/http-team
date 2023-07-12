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
 * FILE NAME  : M1_L1_LCDAnimation.c      
 * DESCRIPTION: This code createds various custom characters
                and then steps through them with 3 second delays.
 **************************************************/
#include "mraa_beaglebone_pinmap.h"

#define LCD_ADDR 		 0x3e

mraa_i2c_context i2cp;

void LCD_init (void)
{
	uint8_t init1[2] = {0x00,0x38};
	uint8_t init2[8] = {0x00, 0x39, 0x14,0x74,0x54,0x6f,0x0c,0x01};
		
	// 2 lines 8 bit 3.3V Version
        mraa_i2c_write(i2cp, init1, 2); //Set for command entry
        mraa_i2c_write(i2cp, init2,8);  //Initialize the LCD display
	usleep(1000);
	mraa_i2c_write(i2cp, init1, 2);
	usleep(1000);


}

void Init_Custom_Char(void)          /* Initialize custom character set.
                                      * Note: Starts at address 0x40 then each 8 bytes a character.
                                      * the last 5 bits define which pixels are on.  
                                       */									  
{
	uint8_t init1[2] = {0x00,0x38};  // Set to command mode
	uint8_t init3[2] = {0x00,0x40};  // Character 0  Top half pixels on
	uint8_t init4[9] = {0x40,
						0b11111,
						0b11111,
						0b11111,
						0b11111,
						0b00000,
						0b00000,
						0b00000,
						0b00000};
	
	uint8_t init5[2] = {0x00,0x48};  // Character 1   Bottom half pixels on
	uint8_t init6[9] = {0x40,
						0b00000,
						0b00000,
						0b00000,
						0b00000,
						0b11111,
						0b11111,
						0b11111,
						0b11111};
	
	uint8_t init7[2] = {0x00,0x50};  // Character 2 Left hand top square
	uint8_t init8[9] = {0x40,
						0b11100,
						0b11100,
						0b11100,
						0b11100,
						0b00111,
						0b00111,
						0b00111,
						0b00111};

	uint8_t init9[2] = {0x00,0x58};  // Character 3 Right Hand top square
	uint8_t init10[9] = {0x40,
						0b00111,
						0b00111,
						0b00111,
						0b00111,
						0b11100,
						0b11100,
						0b11100,
						0b11100};
	uint8_t init11[2] = {0x00,0x60};  // Character 4 all pixels on
	uint8_t init12[9] = {0x40,
						0b11111,
						0b11111,
						0b11111,
						0b11111,
						0b11111,
						0b11111,
						0b11111,
						0b11111};
						
	//Set up Custom Character RAM with the above tables 
	mraa_i2c_write(i2cp, init3, 2);
	usleep(1000);
	mraa_i2c_write(i2cp, init4, 9);
	usleep(1000);

    mraa_i2c_write(i2cp, init5, 2);
	usleep(1000);
	mraa_i2c_write(i2cp, init6, 9);
	usleep(1000);

	mraa_i2c_write(i2cp, init7, 2);
	usleep(1000);
	mraa_i2c_write(i2cp, init8, 9);
	usleep(1000);

	mraa_i2c_write(i2cp, init9, 2);
	usleep(1000);
	mraa_i2c_write(i2cp, init10, 9);
	usleep(1000);

	mraa_i2c_write(i2cp, init11, 2);
	usleep(1000);
	mraa_i2c_write(i2cp, init12, 9);
	usleep(1000);

}

void Fill_Display_With_Char(uint8_t char_display)
{
	uint8_t i;
	uint8_t init_line1[3] = {0x00,0x80}; // Set cursor to start of line 1
	uint8_t init_line2[3] = {0x00,0xc0}; // Set cusor to start of line 2
	uint8_t line_buff[17] = {0x40, 0x00}; // Buffer for character write, first byte is 0x40 then 16 character buffer.
	
	for (i = 0; i < 0x10; i ++)  // Copy character to display to all the vlauers in the buffer.
		{
		line_buff[i+1] = char_display;
		}
	mraa_i2c_write(i2cp, init_line1, 2);
	mraa_i2c_write(i2cp, line_buff, 17);
	usleep(1000);
	mraa_i2c_write(i2cp, init_line2, 2);
	mraa_i2c_write(i2cp, line_buff, 17);
	usleep(1000);
}

void Draw_Full_Blocks(void)
{
	
			uint8_t init4[2] = {0x00,0x01}; // Clear display
            uint8_t char0 = 0x00;
            uint8_t char1 = 0x01;
            uint8_t char2 = 0x02;
            uint8_t char3 = 0x03;
            uint8_t char4 = 0x04;			
						
            Fill_Display_With_Char(char0);
			sleep (3);
						
		    Fill_Display_With_Char(char1);
			sleep (3);

            Fill_Display_With_Char(char2);
			sleep (3);
						
            Fill_Display_With_Char(char3);
			sleep (3);
						
            Fill_Display_With_Char(char4);
			sleep (3);
						
			mraa_i2c_write(i2cp, init4, 2);
			
			sleep (1);		
}


int main()
{
	int status;

// Initialize and check the I2C Display
	i2cp = mraa_i2c_init_raw (I2CP_BUS);
	if (!i2cp) { // The bus is not there 
	   printf("I2C Bus 2 is not available. Please check /dev/i2c-2 \n");
	   printf(" -> Exiting program\n");
	   mraa_i2c_stop(i2cp);
	   mraa_deinit();
	   return EXIT_FAILURE;
	}

	mraa_i2c_frequency (i2cp, MRAA_I2C_STD);
	mraa_i2c_address(i2cp, LCD_ADDR);
	status=mraa_i2c_read_byte(i2cp);
	if (status < 0){
            printf("Failed to initialize display\n");
	    printf("Check i2cdetect -r -y 2 for 0x3E\n");
            mraa_i2c_stop(i2cp);
            mraa_deinit();
            return EXIT_FAILURE;
        }
	
	LCD_init();	
	Init_Custom_Char();
	sleep(1);
	printf("LCD Animation Program Running...Hit Control-C to Exit.\n");
	while (1)
	{
		Draw_Full_Blocks();
		sleep(1);
	}
}
