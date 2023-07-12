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
# FILE NAME  :  M2_L7_T1_password_checker.py      
# DESCRIPTION:  This program compares password using
#               plain text.
###################################################

password = "Keysight U3810A"

pass_phrase = raw_input("Enter Password >")
if (pass_phrase == password):
	print "Password is correct"
else:
	print "Password is incorrect"

