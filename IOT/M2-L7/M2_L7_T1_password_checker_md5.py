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
# FILE NAME  :  M2_L7_T1_password_checker_md5.python      
# DESCRIPTION:  This program compares the password
#               entered as plain text against the
#				md5 hashed phrase.
###################################################
# MS5 Password Checker

import hashlib

password = "03e3524785047613ba7cbee21f616910"

pass_phrase = raw_input("Enter Password >")
result=hashlib.md5(pass_phrase)
if (result.hexdigest() == password):
	print "Password is correct"
else:
	print "Password is incorrect"

