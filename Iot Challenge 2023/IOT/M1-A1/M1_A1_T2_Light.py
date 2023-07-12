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
# FILE NAME  :  M1_A1_T2_Light.py      
# DESCRIPTION:  This program retrieves LUX data  
#               from SensorTag and calls for the 
#				lcd executable to switch the Relay
#				according to threshold value.
# NOTE       :  Replace the SensorTag address
#				with your SensorTag's MAC address
# #################################################

import os
import sys
import time
from bluepy import sensortag

os.system("./clear_lcd")

print "\nPreparing to connect..."
print "You might need to press the side button on Sensor Tag within 2 seconds..."
time.sleep(1.0)

# Initializes SensorTag and threshold values
tag = sensortag.SensorTag('24:71:89:BC:19:03')
luxThreshold = 250.0
switch = 0
tag.lightmeter.enable()

while (1):
	tag.waitForNotifications(1.0)
	data = tag.lightmeter.read()
	print "Lux: ", data
	
	output = os.system("./button")	# Acts as overwrite button
	if (abs(data) < luxThreshold) and (output==0):
		if (switch==0):		# Disables switching if switch is already in wanted state
			os.system("./light 1")
			os.system("./lcd Lights_ON")
			switch = 1
	elif (switch==1):		# Disables switching if switch is already in wanted state
		os.system("./light 0")
		os.system("./lcd Lights_OFF")
		switch = 0
	time.sleep(1.0)
tag.disconnect()
del tag