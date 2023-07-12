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
# FILE NAME  : M1_L2_E5b_Cloud_LCD.py      
# DESCRIPTION: This program gets weather data from
#			   the internet and prints it to the LCD
# #################################################
import os
import sys
import time

while (1):

	os.system("node /home/debian/weather_lcd.js")
	time.sleep (5)