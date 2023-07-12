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
# FILE NAME  :  LCD_basic.py      
# DESCRIPTION:  This program displays any Text to the LCD.
#  This calls the LCD as a function
# #################################################
import LCD_Fun as LCD

LCD.LCD_init()   # Initialize the LCD
while True:
	instr = raw_input(" Input Text for display >")
	LCD.LCD_clear()
	if len(instr) <= 20:
		LCD.LCD_print(instr)
	else:
		LCD.LCD_print(instr[:20])
		

