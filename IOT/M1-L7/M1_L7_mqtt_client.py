#!/usr/bin/python 

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
# FILE NAME  :  M1_L7_mqtt_client.py      
# DESCRIPTION:  This program subscribe to an MQTT
#               broker and to print out incoming
#               messages.
# NOTE       :  Replace the 'broker' address with 
#               your Mosquitto broker IP address.
# #################################################

import sys
import paho.mqtt.client as mqtt

broker = "127.0.0.1"
topic = "mytopic"

# The callback for when the client receives a CONNACK response from  # the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
# Subscribing in on_connect() means that if we lose the      
# connection and reconnect then subscriptions will be renewed.
    client.subscribe(topic)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(broker, 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks # and handles reconnecting.
client.loop_forever()
