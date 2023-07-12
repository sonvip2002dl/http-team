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
# FILE NAME  :  M2_L7_T3_cypher_3d.py      
# DESCRIPTION: This program encrypts a text using
# 	       decrypts input using wheel with opposite 
#              motion of the encryption wheel
# #################################################
import random

wheel_1 = "yzdrkxcojaqhbveftsuwigplnm"  # default wheel
offset = input("Enter offset: ")
offset = (26 -offset)%26                # Flip the offset

str_in = raw_input("Input text >")
str_low = str_in.lower()			# Convert to lower case
cypher_text = ""
for ch in str_low:
	if ch.isalpha():
		i = wheel_1.find(ch)
		ch = wheel_1[(i + offset)%26]
                offset -= 1
	cypher_text += ch
print cypher_text
