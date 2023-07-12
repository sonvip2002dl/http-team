#!/usr/bin/python 
# coding: utf8

#################################################
#         © Keysight Technologies 2019 
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
# FILE NAME  :  M1_L5_T3b_Light_LCD.py      
# DESCRIPTION:  This program retrieves lightmeter 
#               data from SensorTag, and displays
#				it on the Putty window and U3811A
#				LCD display.
# NOTE       :  Replace the SensorTag address
#				with your SensorTag's MAC address
#
# #################################################

import os
import sys
import time
import M1_L5_LCD_Fun as LCD
from bluepy import sensortag

LCD.LCD_init()
LCD.LCD_clear()

print "\nPreparing to connect..."
print "You might need to press the side button on Sensor Tag within 2 seconds..."
time.sleep(2.0)

tag = sensortag.SensorTag('54:6C:0E:79:01:87')
tag.lightmeter.enable()

while (1):
	tag.waitForNotifications(1.0)
	data = tag.lightmeter.read()
	lux = data
	print lux
	disp = "{:3.0f}".format(lux) + " Lux"
	LCD.LCD_print2(disp)
	time.sleep(1.0)
tag.disconnect()
del tag