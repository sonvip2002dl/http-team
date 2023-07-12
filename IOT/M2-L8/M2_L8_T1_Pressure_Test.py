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
# FILE NAME  :  Pressure_Test.py      
# DESCRIPTION:  This program displays the Pressure
# and temperature on the LCD display using python calls.
###################################################
#
import time
import LCD_Fun as LCD
import Pressure_Fun as Pres


LCD.LCD_init()   # Initialize the LCD
if( not Pres.Pressure_init()):
    print "Pressure Sensor initialization error.  Exiting program"
    exit()
LCD.LCD_clear()

while True:
        Pressure_Data = Pres.Pressure_read()
        LCD.LCD_print("Pressure  Temp")
        pressure = str(Pressure_Data[0])
        temp = str(Pressure_Data[2])
        if (1000.00 <= Pressure_Data[0]):
            LCD.LCD_print2(pressure + "mB  " + temp+"C   ")
        else:
            LCD.LCD_print2(pressure + " mB  " + temp+"C  ")
        time.sleep(1)

