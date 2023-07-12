# -*- coding: utf-8 -*-
import Adafruit_BBIO.GPIO as GPIO
import time
import requests

channel_url = "https://api.thingspeak.com/channels/1956999/field/1/last"

output_pin = "P9_41"
GPIO.setup(output_pin, GPIO.OUT)

def read_data_from_thingspeak():
    try:
        response = requests.get(channel_url)
        data = response.json()
        if "field1" in data:
            return int(data["field1"])
    except:
        pass
    return None

def set_output_state(state):
    if state == 0:
        GPIO.output(output_pin, GPIO.LOW)
    elif state == 1:
        GPIO.output(output_pin, GPIO.HIGH)

print "Reading\t\tOutput State"
while True:
    value = read_data_from_thingspeak()
    if value is not None:
        print str(value) + "\t\t",
        set_output_state(value)
    else:
        print "Error reading data\t",
    
    time.sleep(1)
