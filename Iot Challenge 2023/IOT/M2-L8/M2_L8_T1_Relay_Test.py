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
# FILE NAME  :  Relay_test.py      
# DESCRIPTION:  This program reads the buttons and
# turns on relay 1 if button 1 is pressed, turns
# off relay 1 if button 2 is pressed.
# If button 3 is pressed it turns on relay 3
# if button 4 is pressed, it turns off relay 2
###################################################
#
import mraa
import time
import LCD_Fun as LCD
from adcPythonC import *
import mraa_beaglbone_pinmap as pm

adc_channel = 7 # Input pin for ADC Channel
buttons = []
button_pins = [pm.B1,pm.B2,pm.B3,pm.B4]
for button in button_pins:
    buttons.append(mraa.Gpio(button))
relay0_state = 0
relay_pins = [pm.GP2,pm.GP3]
relays=[]
for r in relay_pins:
    relays.append(mraa.Gpio(r))
for button in buttons:
    button.dir(mraa.DIR_IN)
for relay in relays:
    time.sleep(1)   # BB requires 1 second pause befor changing direction!
    r = relay.dir(mraa.DIR_OUT)
    relay.write(0)
LCD.LCD_init()
relay0_state = 0
relay1_state = 0
LCD.LCD_clear()
while True:
    if(relay0_state == 0):  # Note Low = on and High = off
        relays[0].write(1)
	LCD.LCD_print("Relay 0 Off  ")
    if(relay0_state  == 1 ):
        relays[0].write(0)
	LCD.LCD_print("Relay 0 On    ")
    if(relay1_state == 0):  # Note Low = on and High = off
        relays[1].write(1)
	LCD.LCD_print2("Relay 1 Off  ")
    if(relay1_state  == 1 ):
        relays[1].write(0)
	LCD.LCD_print2("Relay 1 On    ")
    if(buttons[0].read() == 0):
    	relay0_state = 1
    if(buttons[1].read() == 0):
    	relay0_state = 0
    if(buttons[2].read() == 0):
    	relay1_state = 1
    if(buttons[3].read() == 0):
    	relay1_state = 0
