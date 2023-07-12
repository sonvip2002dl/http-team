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
# FILE NAME  :  M1_A2_T2_Machine.py      
# DESCRIPTION:  This program retrieves accelerometer 
#               data from SensorTag, and changes 
#				the PWM signal if the data
#				exceeds the threshold. This program 
#				also updates the Cloud every 
#				10 readings and allows a button to
#				overriding the alarm.
# NOTE       :  Replace the SensorTag address
#				with your SensorTag's MAC address
#
# #################################################

import os
import sys
import time
from bluepy import sensortag

os.system("./pwm 0.001")

print "\nPreparing to connect..."
print "You might need to press the side button on Sensor Tag within 2 seconds..."
time.sleep(2.0)

tag = sensortag.SensorTag('24:71:89:BC:19:03')	# Replace this with your SensorTag address
#localhost = "192.168.7.2"
accThreshold = 3.0;
tag.accelerometer.enable()
counter = 1

while (1):
	tag.waitForNotifications(1.0)
	data = tag.accelerometer.read()
	total = abs(data[0])+abs(data[1])+abs(data[2])
	print total
	output = os.system("./button")	# Acts as overwrite button
	if (total == 0.0):
			continue
	# Compare current temperature with threshold
	if (total > accThreshold) and (output==0):
			target = open("accFile",'w')
			target.close()
			os.system("./pwm 0.5")
	else:
			os.system("./pwm 0.001")
	# Update to Cloud
	counter+=1
	if (counter > 10):
		counter = 0
		os.system("/home/debian/LabCode/M1-A2/update-google.sh " + str(data[0]) + " " + str(data[1]) + " " + str(data[2]))
	time.sleep(1.0)
tag.disconnect()
del tag