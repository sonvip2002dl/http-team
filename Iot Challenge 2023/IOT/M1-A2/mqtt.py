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
# FILE NAME  :  mqtt.py      
# DESCRIPTION:  This program publishes a message
#				to MQTT.
# NOTE       :  Replace the 'broker' address with 
#               your Mosquitto broker IP address 
#				
# #################################################

import os
import sys
import time
from bluepy import sensortag
import paho.mqtt.client as mqtt

# replace with the mosquitto broker's IP address
broker = '192.168.7.1'  # PC address via RDNiS
port = 1883
topic = "/mytopic"
client = mqtt.Client()
client.connect(broker, port)
		
msg = sys.argv[1]
client.publish(topic, msg)
