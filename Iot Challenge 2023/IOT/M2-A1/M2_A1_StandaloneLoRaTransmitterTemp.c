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
 * FILE NAME  : StandaloneLoRaTransmitterTemp.c
 * DESCRIPTION: Display the DIGITAL TEMP SENSOR value 
 *              to the LCD display and transmit via
 *              LoRa XCVR.  
 **************************************************/
#include "mraa_beaglebone_pinmap.h"

#define LCD_ADDR 		 0x3E
#define LM75A_I2C_ADDR        0x48

int ADC_Chan = 7;             // Default ADC Channel on MCP3208
mraa_i2c_context i2cp;
mraa_i2c_context I2Chandle;
mraa_gpio_context MCP3208_DIN;
mraa_gpio_context MCP3208_DOUT;
mraa_gpio_context MCP3208_CLK;
mraa_gpio_context MCP3208_CS;

mraa_uart_context uart;

void delay_time (unsigned int time_end){
	unsigned int index;
	for (index = 0; index < time_end; index++);
}

void send_cmd (char* cmd){
	char* temp = cmd;
	for (; *temp != '\0'; temp ++)
	{
		mraa_uart_write(uart, temp, 1);
	}
	mraa_uart_write(uart, "\r\n", 2);
	delay_time (300);
	mraa_uart_flush(uart);
	char rep[20] = "";
	mraa_uart_read(uart,rep,20);
	//printf ("%s: %s\n",cmd,rep);
}

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

void LCD_Print2 (uint8_t* str)
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

int GetMCP3208 (int Channel);

int main()
{
	char buf [20];
	int status;
	float T = 0;
	unsigned int x = 0;
	unsigned char i2c_buf [2] = {0, 0};
	uart= NULL;
	mraa_init();
	char dev_string[] = "/dev/ttyS2";

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
	printf("Status = %d",status);
	if (status < 0){
         printf("Failed to initialize display\n");
		 printf("Check i2cdetect -r -y 2 for 0x3E\n");
         mraa_i2c_stop(i2cp);
         mraa_deinit();
         return EXIT_FAILURE;
        }
		
    I2Chandle = mraa_i2c_init_raw (I2CS_BUS); // write correct I2C dev number here
	if (!I2Chandle) { // The bus is not there 
		printf("I2C Bus 1 is not available. Please check /dev/i2c-2 \n");
		printf(" -> Exiting program\n");
		mraa_i2c_stop(I2Chandle);
		mraa_deinit();
		return EXIT_FAILURE;
	}
    mraa_i2c_frequency (I2Chandle, MRAA_I2C_STD);
	mraa_i2c_address(I2Chandle, LM75A_I2C_ADDR);
	status = mraa_i2c_read_byte(I2Chandle);
	if (status < 0){
         printf("Failed to Initialize Digital Temprature Sensor -> Exiting program\n");
		 printf("Check i2cdetect -r -y 2 for 0x48\n");
         mraa_i2c_stop(I2Chandle);
         mraa_deinit();
         return EXIT_FAILURE;
    }

	
	//Initialize SPI bus
	MCP3208_DIN = mraa_gpio_init (SPI_MOSI_PIN);
	MCP3208_DOUT = mraa_gpio_init (SPI_MISO_PIN);
	MCP3208_CLK = mraa_gpio_init (SPI_CLK_PIN);
	MCP3208_CS = mraa_gpio_init (SPI_CS0_PIN);
    sleep(1); // Need a delay before setting the direction.	
	mraa_gpio_dir(MCP3208_DIN, MRAA_GPIO_OUT_HIGH);
	mraa_gpio_dir(MCP3208_DOUT, MRAA_GPIO_IN);
	mraa_gpio_dir(MCP3208_CLK, MRAA_GPIO_OUT);
	mraa_gpio_dir(MCP3208_CS, MRAA_GPIO_OUT);

	
    LCD_init();
	status = mraa_i2c_read_byte(I2Chandle);
	if (status < 0){
	     printf("Failed to Initialize Temperature Sensor -> Exiting program\n");
	     mraa_i2c_stop(I2Chandle);
	     mraa_deinit();
	     return EXIT_FAILURE;
             }

	uart = mraa_uart_init_raw(dev_string);
	if(uart == NULL)
	{
		printf("Failed to setup UART\n");
		return 0;
	}

	mraa_uart_set_baudrate(uart, 57600);
	mraa_uart_set_mode(uart, 8, MRAA_UART_PARITY_NONE, 1);
	mraa_uart_set_flowcontrol(uart, 0, 0);
	mraa_uart_set_timeout(uart, 0, 0, 0);

	printf("\nTransmitting data..\n");
	send_cmd((char*)"radio cw off");
	sleep(2);
	send_cmd((char*)"radio set freq 433050000");
	
	while (1)
	{
		LCD_Print ((uint8_t*)buf);
		
		mraa_i2c_read_bytes_data(I2Chandle,0x00, i2c_buf, 2);  //added Reg 0x00

		x = (i2c_buf [0] << 8) | i2c_buf [1];
		x = (x & 0x7FFF) >> 7;
		T = x / 2.0;
		if (i2c_buf [0] & 0x80)
		{
			sprintf(buf, "radio tx -%2.0f", T, 0xDF);
		}
		else
		{
			sprintf(buf, "radio tx %2.0f", T,0xDF);
		}
		LCD_Print2((uint8_t*)buf);
		printf("%s\n", buf);
						
		send_cmd((char*)"mac pause");
		send_cmd((char*) buf);
		sleep(5);
	}
	
	return MRAA_SUCCESS;
}

int GetMCP3208 (int Channel)
{
	int i;
	int val;
	
	mraa_gpio_write (MCP3208_DIN, 0);
	mraa_gpio_write (MCP3208_CLK, 0);
	mraa_gpio_write (MCP3208_CS, 0);

	Channel = Channel | 0x18;
	for (i = 0; i < 5; i ++)
	{
		if (Channel & 0x10)
		{
			mraa_gpio_write (MCP3208_DIN, 1);
		}
		else
		{
			mraa_gpio_write (MCP3208_DIN, 0);
		}
		Channel <<= 1;

		mraa_gpio_write (MCP3208_CLK, 0);
		mraa_gpio_write (MCP3208_CLK, 1);
	}

	mraa_gpio_write (MCP3208_CLK, 0);
	mraa_gpio_write (MCP3208_CLK, 1);

	mraa_gpio_write (MCP3208_CLK, 0);
	mraa_gpio_write (MCP3208_CLK, 1);

	val = 0;
	for (i = 0; i < 12; i ++)
	{
		mraa_gpio_write (MCP3208_CLK, 0);
		mraa_gpio_write (MCP3208_CLK, 1);
		
		val = (val << 1) | ((int) mraa_gpio_read (MCP3208_DOUT));	
	}
	
	mraa_gpio_write (MCP3208_CS, 1);
	mraa_gpio_write (MCP3208_DIN, 0);
	mraa_gpio_write (MCP3208_CLK, 0);

	return val;
}
