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
# FILE NAME  :  M2_L7_T3_cypher_4d.py      
# DESCRIPTION: This program decrypts a text using
# 			   a 3 wheel character subsitution.
# #################################################
import random

wheel_1 = "yzdrkxcojaqhbveftsuwigplnm"  # default wheel
wheel_2 = "vatmrlbzjfscdwyuopnegxhqik"
wheel_3 = "bdguirftmqxhnpayvwkljcesoz"
offset = input("Enter offset: ")
offset = (26-offset)%26
str_in = raw_input("Input text >")
str_low = str_in.lower()			# Convert to lower case
cypher_text = ""
for ch in str_low:
	if ch.isalpha():
		i = wheel_3.find(ch)
		ch = wheel_3[(i + offset)%26]
		i = wheel_2.find(ch)
		ch = wheel_2[(i + offset)%26]
		i = wheel_1.find(ch)
		ch = wheel_1[(i + offset)%26]
		offset -= 1
	cypher_text += ch

print cypher_text
