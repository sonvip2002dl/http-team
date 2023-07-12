#!/usr/bin/python 

#################################################
#         (C) Keysight Technologies 2020 
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
# FILE NAME  :  M1_L7_Secure_Pressure_Post.py  [ip_address] [Loop Count]       
# DESCRIPTION:  This program calls 
#		M1_L7_Pressure_Fun.Press_read() function
#               and publish the readings to mqtt as well
#		as google script
#		This uses the ca.crt created by the student
#		in /debian/home/ca.crt
#
# NOTE       :  Replace the 'broker' address with 
#               your Mosquitto broker IP address; and the
#		serviceUrl to your google script URL.
# #################################################
# Distributed with a free-will license.
#
#This is basic functions for the LCD display 
import time
import sys
import os
import mraa
import M1_L7_LCD_Fun as LCD
import M1_L7_Pressure_Fun as Pres
import paho.mqtt.client as mqtt

max_retries = 2   # Maximum ping retries before exiting

if len(sys.argv) > 1:
    broker = sys.argv[1]
else:
    broker = "192.168.7.1"  # Default RNDiS address
    print "WARNING The Default address for Secure MQTT may not work"
    print "Please restart with Computer Name in the Certificate"

if len(sys.argv) > 2:
    loop_count = int(sys.argv[2]) # Optional loop count 
else:
    loop_count = -1


LCD.LCD_init()   # Initialize the LCD
if( not Pres.Pressure_init()):
    print "Pressure Sensor initialization error.  Exiting program"
    exit()
chk_broker = os.system("ping -c 1 " + broker)
retries = 0
while chk_broker != 0:
  print broker, 'is not up, retrying...'
  retries += 1
  if retries > max_retries:
     print 'Maximum Retry count Exceeded'
     print 'Check the broker ip address'
     exit()
  time.sleep(2)
  chk_broker = os.system("ping -c 1 " + broker)

port = 8883
topic = "/mytopic"
tls_cert="/home/debian/ca.crt"
client = mqtt.Client()
client.tls_set(tls_cert)
client.connect(broker, port)

LCD.LCD_clear()
while True:
	Pressure_Data=Pres.Pressure_read()
	LCD.LCD_print("Pressure  Temp  ")
	pressure = str(Pressure_Data[0])
	temp = str(Pressure_Data[2])
        if (1000.00 <= Pressure_Data[0]):
	    LCD.LCD_print2(pressure + "mB  " + temp+"C  ")
        elif (300.0 < Pressure_Data[0]):
	    LCD.LCD_print2(pressure + " mB  " + temp+"C  ")
	params = (
		('Temp', temp),
		('Humid', pressure)
	)
	msg = "Pressure = " + pressure + " mB, Temperature = " + temp + " C"
	client.publish(topic, msg)
	time.sleep(2)
