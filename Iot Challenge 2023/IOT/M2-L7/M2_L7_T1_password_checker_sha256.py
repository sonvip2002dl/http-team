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
# FILE NAME  :  M2_L7_T1_password_checker_sha256.python      
# DESCRIPTION:  This program compares the password
#               entered as plain text against the
#				sha256 hashed phrase.
###################################################
# SHA256 Password Checker

import hashlib

password = "422aef3e7d92233a8337d4a04d4e109983407f6c79a4880ad9a9459de161d6ee"

pass_phrase = raw_input("Enter Password >")
result=hashlib.sha256(pass_phrase)
if (result.hexdigest() == password):
	print "Password is correct"
else:
	print "Password is incorrect"

