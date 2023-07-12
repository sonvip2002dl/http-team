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
# FILE NAME  :  M1_A1_T1_IOT_Applications.py      
# DESCRIPTION:  This program retrieves accelerometer 
#               data from SensorTag, and calls
#				M1_A1_T1_weather.js if the data
#				exceeds the threshold.
# NOTE       :  Replace the SensorTag address
#				with your SensorTag's MAC address
#
# #################################################

import os
import sys
import time
from bluepy import sensortag

os.system("./clear_lcd")

print "\nPreparing to connect..."
print "You might need to press the side button on Sensor Tag within 2 seconds..."
time.sleep(2.0)

tag = sensortag.SensorTag('24:71:89:BC:19:03')
#localhost = "192.168.7.2"
accThreshold = 3.0;
tag.accelerometer.enable()

while (1):
	tag.waitForNotifications(1.0)
	data = tag.accelerometer.read()
	total = abs(data[0])+abs(data[1])+abs(data[2])
	print total
	if (total == 0.0):
			continue
	if (total > accThreshold):
			target = open("accFile",'w')
			target.close()
			#os.system("/home/root/update-google.sh " + str(total[0]))
			os.system("node /home/debian/LabCode/M1-A1/M1_A1_T1_weather.js")
	time.sleep(1.0)
tag.disconnect()
del tag