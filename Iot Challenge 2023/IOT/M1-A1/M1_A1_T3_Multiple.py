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
# FILE NAME  :  M1_A1_T3_Multiple.py      
# DESCRIPTION:  This program retrieves data from  
#               SensorTag, logs the data in a
#				text file and publishes the  
#				readings to MQTT.
# NOTE       :  Replace the 'broker' address with 
#               your Mosquitto broker IP address, 
#				and replace the SensorTag address
#				with your SensorTag's MAC address
# #################################################

import os
import sys
import time
from bluepy import sensortag
import paho.mqtt.client as mqtt

os.system("./clear_lcd")

# replace with the mosquitto broker's IP address
broker = '192.168.7.1'  # PC address via RDNiS
port = 1883
topic = "/mytopic"
client = mqtt.Client()
client.connect(broker, port)

print "\nPreparing to connect..."
print "You might need to press the side button on Sensor Tag within 2 seconds..."
time.sleep(1.0)

# Initializes SensorTag and threshold values
tag = sensortag.SensorTag('24:71:89:BC:19:03')
tag.lightmeter.enable()
tag.humidity.enable()
tag.barometer.enable()

count=0;		# 5 mins is #counts
hourcount=0;	
temp=0;
htemp=0;
humi=0;
hhumi=0;

print "Connected"

while (1):
	tag.waitForNotifications(1.0)
	dataLux = tag.lightmeter.read()
	dataHumidity = tag.humidity.read()
	dataPressure = tag.barometer.read()
	
	count = count + 1

	if (count > 4):		# 5 counts is 5 mins
		temp=temp+dataHumidity[0]
		humi=humi+dataHumidity[1]
		time.sleep(1.0)
		count = 0
		hourcount = hourcount + 1
		cmd = "./log %s %s %s %s" % (str(dataHumidity[0]), 
			str(dataLux), str(dataHumidity[1]), str(dataPressure[1]))
		os.system(cmd)
		print
		print "Temperature: ", dataHumidity[0], "'C"
		print "Lux:         ", dataLux, " lx"
		print "Humidity:    ", dataHumidity[1], " g/m3"
		print "Pressure:    ", dataPressure[1], " Pa"
		
		msg = "Temp: %3.1f'C\nLux: %3.1f lx\nHumidity: %3.1f g/m3\nPressure: %3.1f Pa\n" % (dataHumidity[0], dataLux, dataHumidity[1], dataPressure[1])
		client.publish(topic, msg)

	if (hourcount > 11): # 12 times of 5 mins is 1 hour
		htemp=temp/12
		hhumi=humi/12
		temp = 0
		humi = 0
		hourcount = 0
		print "Avg Temp, Avg Humi: ", htemp, ",", hhumi
	
tag.disconnect()
del tag