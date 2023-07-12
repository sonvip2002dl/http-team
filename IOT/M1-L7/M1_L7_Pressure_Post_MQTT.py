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
# FILE NAME  :  M1_L7_Pressure_Post_MQTT.py  [ip_address] [Loop Count]    
# DESCRIPTION:  This program calls 
#				M1_L7_Pressure_Fun.Press_read() 
#               function and publish the readings 
#				to mqtt.
# NOTE       :  default ip_address is 192.168.7.1 default RNDiS
#		default loop count is -1 which will be an infinite loop
#		This program will ping the broker address before statring.
#		It will retry mxa_retries times before exiting with a 2 second
#		delay between retries
# #################################################
# Distributed with a free-will license.
# #################################################
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

  

port = 1883
topic = "/mytopic"
client = mqtt.Client()
client.connect(broker, port)

# Everyth1ng is ready to go now Clear the LCD and start up the loop
LCD.LCD_clear()

while (loop_count != 0):
	Pressure_Data=Pres.Pressure_read()
	LCD.LCD_print("Pressure  Temp  ")
	pressure = str(Pressure_Data[0])
	temp = str(Pressure_Data[2])
        if (1000.00 <= Pressure_Data[0]):
	    LCD.LCD_print2(pressure + "mB  " + temp+"C  ")
        elif (300.0 < Pressure_Data[0]):
	    LCD.LCD_print2(pressure + " mB  " + temp+"C  ")
	msg = "Pressure = " + pressure + " mB, Temperature = " + temp + " C"
	client.publish(topic, msg)
	print(msg)
	time.sleep(2)
        loop_count -= 1
