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
# FILE NAME  :  M3_L7_Temp_Post_MQTT.py  [ip_address] [Loop Count]    
# DESCRIPTION:  This program calls 
#				M3_L7_LCD_Fun.py to print to LCD
#               function and publishes the readings 
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
import M3_L7_LCD_Fun as LCD
import paho.mqtt.client as mqtt

max_retries = 2   # Maximum ping retries before exiting
LM75A_I2C_ADDR = 0x48

def Temp_Read():
    data = bus.read(2)    # read 2 bytes of the temperature sensor
    temp = int(data[0])
    if temp > 127:        # Check sign of temperature flag
       temp = temp - 128  # if sign set, remove it
       temp = temp + float(data[1])/256.0
       temp = - temp
    else:
       temp = temp + float(data[1])/256.0
    return(temp)


if len(sys.argv) > 0:
    broker = sys.argv[1]
else:
    broker = "192.168.7.1"  # Default RNDiS address

if len(sys.argv) > 1:
    loop_count = int(sys.argv[2]) # Optional loop count 
else:
    loop_count = -1


LCD.LCD_init()   # Initialize the LCD
bus = mraa.I2c(1,True)
status = bus.address(LM75A_I2C_ADDR)
if (status != 0):
    print "Temperature Sensor Error, Please check for 0x48 on i2cdetect -r -y 1"
    exit()
try:
    data = bus.read(2)                # Do one read to make sure it is there
except:
    print "Temperature Sensor Error, Please check for 0x48 on i2cdetect -r -y 1"
    exit()


# replace with the mosquitto broker's IP address
chk_broker = os.system("ping -c 1 " + broker)
retries = 0
while chk_broker != 0:
  print broker, 'is not up, retrying...'
  retries += 1
  if retries > max_retries:
     print 'Maximum Retry count Exceeded'
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
	LCD.LCD_print("Temp  ")
	temp_str = str(Temp_Read())
        LCD.LCD_print("Temp = " + temp_str + "C   ")
	msg = "Temperature = " + temp_str + " C"
	client.publish(topic, msg)
	time.sleep(2)
        loop_count -= 1
