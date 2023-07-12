/* BeagleBone Pin Mappings for NB Board
 *
 * revision 0.1 - Initial revision with proposed mappings
 * revision 0.2 - Set I2CP bus for private bus (EEPROM and Display)
 * revision 0.3 - Correccted pin map
 * revision 0.4 - Added SPI_BUS, SPI_FREQ
 * revision 0.5 - Added Button B1,B2,B3,B4
 * revision 0.6 - Updated for LP2 version
 *
 * GP2 to GP9 go to Arduino Connector
 * GP7 Connected to Button #1
 * GP8 Connected to Button #2
 * GP9 Connected to Button #3
 * GPB4 Connected to Button #4
 *
 * SPI_CS0 only goes to ADC 3208 (AKA SPI_DEV2.0)
 * SPI control signals goto user connectors gated by SPI_CS0
 *
 * I2CP bus goes to EEPROM and Display - Consider as Private Bus
 *
 * I2C1 bus goes to Arduino Connector and user connectors
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "mraa.h"

#define HIGH  1
#define LOW   0

#define TRUE  1
#define FALSE 0


// mraa pin numbers
// Note for BeagleBone  The signals are numbered by the J8 and J9 pin  use "mraa-gpio list" to see mapping

//  SPI Bus Controls
int SPI_BUS             = 1;    //SPI Bus 1 (AKA SPIDEV2.x)
int SPI_FREQ            = 1000000; // SPI clock operating frequency
int SPI_MOSI_PIN 		= 76; 	//GPIO3_16
int SPI_MISO_PIN 		= 75; 	//GPIO3_15
int SPI_CLK_PIN 		= 77; 	//GPIO3_14
int SPI_CS0_PIN			= 74; 	//GPIO3_17
int SPI_CS1_PIN         = 88;   //GPIO0_7

// I2C Bus Controls
int I2CS_BUS        = 1;   	//I2C Bus to be used for Sensors
int I2C1_SCL_PIN    = 63;   //GPIO0_5
int I2C1_SDA_PIN    = 64;   //GPIO0_4

int I2CP_BUS        = 2;   	//I2C Bus Bus for Display 
int I2C2_SCL_PIN    = 65;  	//GPIO0_13
int I2C2_SDA_PIN    = 66;  	//GPIO0_12

int RELAY_PIN		= 87;   ////GPIO0_20

// GP - General Purpose io pins
int GP2             = 87; 	//GPIO0_20
int GP3             = 73; 	//GPIO3_19
int GP4             = 71; 	//GPIO3_21
int GP5             = 62;  	//GPIO1_19
int GP6             = 60;  	//GPIO1_18
int GP7             = 69;  	//GPIO1_17
int GP8             = 61;  	//GPIO1_16
int GP9             = 59;  	//GPIO0_31
int GPB4            = 57;  	//GPIO0_30


int UART1 			= 1;

// Buttons
int B1				= 69;
int B2				= 61;
int B3				= 59;
int B4				= 57;