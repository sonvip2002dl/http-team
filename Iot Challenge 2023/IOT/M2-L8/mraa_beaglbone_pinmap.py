#!/usr/bin/python
#################################################
#         (C) Keysight Technologies 2019 
#
# PROPRIETARY RIGHTS of Keysight Technologies are 
# involved in the subject matter of this software. 
# All manufacturing, reproduction, use, and sales 
# rights pertaining to this software are governed 
# by the license agreement. The recipient of this 
# code implicitly accepts the terms of the license. 
#
###################################################
#
# FILE NAME  :  mraa_beaglbone_pinmap.py      
# Beaglebone Pinmapping to variables for U3810A LP2
####################################################
# SPI Bus Controls
SPI_BUS 			= 1
SPI_MOSI_PIN 		= 76 	#GPIO3_16
SPI_MISO_PIN 		= 75 	#GPIO3_15
SPI_CLK_PIN 		= 77 	#GPIO3_14
SPI_CS0_PIN			= 74 	#GPIO3_17
SPI_CS1_PIN         = 88    #GPIO0_7

# I2C Bus Controls
I2CS_BUS = 1
I2CP_BUS = 2

# GPIO Pins
GP2 = 87
GP3 = 73
GP4 = 71
GP5 = 62
GP6 = 60
GP7 = 69
GP8 = 61
GP9 = 59
GPB4 = 57
GP11 = 58

UART1 = 1

# Buttons
B1 = 69
B2 = 61
B3 = 59
B4 = 57
