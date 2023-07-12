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
# FILE NAME  :  M3_L7_LCD_Fun.py      
# DESCRIPTION: This program provides initialization,
# clear, LCD_print for the first line and LCD_print for the
# second line.
# #################################################
#
# This is a Python version that runs the LCD Display using MRAA
#
# This is basic functions for the LCD display 
import time
import mraa
LCD_Addr = 0x3E
# Get I2C bus
bus2 = mraa.I2c(2,True)
bus2.address(LCD_Addr)

def LCD_home():
        buf = [0x00,0x02]
        bus2.write(bytearray(buf))  #Set to Home

def LCD_home2():
	buf=[0x00,0x02,0xC0]
        bus2.write(bytearray(buf))  #Set to  second line


def LCD_print(strin):
        LCD_home()
        buf2 = '@' + strin
        bus2.write(bytearray(buf2))

def LCD_print2(strin):
        LCD_home2()
        buf2 = '@' + strin
        bus2.write(bytearray(buf2))


def LCD_init():

   init1 = [0x00,0x38]
   init2 = [0x00,0x39,0x14,0x74,0x54,0x6f,0x0c,0x01]
   # 2 lines 8 bit 3.3V Version
   bus2.write(bytearray(init1))
   bus2.write(bytearray(init2))


def LCD_clear():
        buf = [0x00,0x01]
        bus2.write(bytearray(buf))  #Clear LCD

LCD_init()   # Initialize the LCD

		

