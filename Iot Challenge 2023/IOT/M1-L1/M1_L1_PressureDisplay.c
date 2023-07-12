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
 * FILE NAME  :  M1_L1_PressureDisplay.c    
 * DESCRIPTION:  This program reads pressure for a 
 *               period pf time and writes values in file.
 *
 **************************************************/

 
#include "mraa_beaglebone_pinmap.h"
#include <time.h>

mraa_i2c_context I2Chandle;
#define Slave_Addr 0x60

#define STATUS     0x00
#define OUT_P_MSB  0x01
#define OUT_P_CSB  0x02
#define OUT_P_LSB  0x03
#define CTRL_REG1  0x26
#define PT_DATA_CFG 0x13
#define LCD_ADDR 		 0x3E
mraa_i2c_context i2cp;


void home_LCD (void)
{
        uint8_t buf[2] = {0x00,0x02};
        mraa_i2c_write(i2cp, buf, 2);  //Set to Home
}
void home2_LCD (void)
{
        uint8_t buf[] = {0x00,0x02,0xC0};
        mraa_i2c_write(i2cp, buf, 3);  //Set to Start of 2nd line 0X40 
}

void LCD_Print2 (char* str)
{
        uint8_t buf[80]={0};  // Set Buffer to all Null
        int i = 0, strl;      
        home2_LCD ();
        buf[i] = 0x40;  //register for display
        i++;
        strl = strlen((char*)str);
        for (int j = 0; j < strl; j++)
        {
               buf[i] = str[j];
               i++;

        }
         mraa_i2c_write(i2cp, buf, i);
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
    mraa_i2c_write(i2cp, init1, 2);
    mraa_i2c_write(i2cp, init2,8);  //Function Set
}

void clear_LCD (void)
{
        uint8_t buf[2] = {0x00,0x01};
        mraa_i2c_write(i2cp, buf, 2);  //Clear Display
}


void toggleOneShot(void)
{

	unsigned char Value = mraa_i2c_read_byte_data(I2Chandle, CTRL_REG1); //Read current settings
	Value &= ~(1 << 1); //Clear OST bit
	mraa_i2c_write_byte_data(I2Chandle, Value, CTRL_REG1);

	Value = mraa_i2c_read_byte_data(I2Chandle, CTRL_REG1); //Read current settings to be safe
	Value |= (1 << 1); //Set OST bit
	mraa_i2c_write_byte_data(I2Chandle, Value, CTRL_REG1);
}


float ReadPressure(void)
{
	unsigned char msb =0, csb=0, lsb=0;
		
	if (mraa_i2c_read_byte_data(I2Chandle, STATUS) & (1 << 2) == 0) 
	{
		toggleOneShot();					//Toggle the OST bit causing the sensor to immediately take another reading
	}

	// Wait for PDR bit, indicates we have new pressure data
	while (mraa_i2c_read_byte_data(I2Chandle, STATUS) & (1 << 2) == 0)
	{
		usleep(10);
	}

	
	msb = mraa_i2c_read_byte_data(I2Chandle, OUT_P_MSB);
	csb = mraa_i2c_read_byte_data(I2Chandle, OUT_P_CSB);
	lsb = mraa_i2c_read_byte_data(I2Chandle, OUT_P_LSB);
	
	toggleOneShot(); //Toggle the OST bit causing the sensor to immediately take another reading

	// Pressure comes back as a left shifted 20 bit number
	long pressure_whole = (long)msb << 16 | (long)csb << 8 | (long)lsb;
	pressure_whole >>= 6;

	lsb &= 0x30; //Bits 5/4 represent the fractional component
	lsb >>= 4; //Get it right aligned
	float pressure_decimal = (float)lsb / 4.0; //Turn it into fraction

	float pressure = (float)pressure_whole + pressure_decimal;

	return(pressure);


}
void SetupPressureSensor(void)
{
	unsigned char tempVal;
	
	mraa_i2c_write_byte_data(I2Chandle, 0x07, PT_DATA_CFG); // Enable all three pressure and temp event flags 
	
	tempVal =  mraa_i2c_read_byte_data(I2Chandle, CTRL_REG1);
	
	// Activate Barometer mode 
	tempVal &= ~(1<<7); 		//Clear ALT bit
	tempVal |= (1<<0); 			//Set SBYB bit for Active mode
	mraa_i2c_write_byte_data(I2Chandle, tempVal, CTRL_REG1);
	
}


int main()
{
	char buf [20];
	
	float pressure =0;
	int status;
	float sample;
	clock_t begin, end;

	struct timespec start, stop;
	double time_elapse;
	
	mraa_init();
	//Initialize Display i2c Bus
        i2cp = mraa_i2c_init_raw (I2CP_BUS);
	mraa_i2c_frequency (i2cp, MRAA_I2C_STD);
	mraa_i2c_address(i2cp, LCD_ADDR);

	I2Chandle = mraa_i2c_init_raw(I2CS_BUS);
	mraa_i2c_frequency(I2Chandle, MRAA_I2C_STD);
	mraa_i2c_address(I2Chandle, Slave_Addr);
	status = mraa_i2c_read_byte(I2Chandle);
	if (status < 0){
	     printf("Failed to Initialize Pressure Sensor -> Exiting program\n");
	     mraa_i2c_stop(I2Chandle);
	     mraa_deinit();
	     return EXIT_FAILURE;
	}
	SetupPressureSensor();
        clear_LCD();	
	
while(1)
	{

		sprintf (buf, "Pressure  ");
		LCD_Print ((uint8_t*)buf);
		pressure = ReadPressure();
		sprintf (buf, "%5.1f mBar  ", pressure/100);
       		//printf("pressure = %5.1f\n",pressure/100);
		LCD_Print2 (buf);
		sleep (1);
	}
	
	return 0;
}

