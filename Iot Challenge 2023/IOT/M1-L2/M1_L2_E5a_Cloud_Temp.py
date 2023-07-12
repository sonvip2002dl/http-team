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
# FILE NAME  : M1_L2_E5a_Cloud_Temp.py      
# DESCRIPTION: This program gets weather data from
#			   the internet
# #################################################
import os
import sys
import time

while (1):

	os.system("node /home/debian/weather_sd.js")
	time.sleep (3)