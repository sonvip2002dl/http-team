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
# FILE NAME  :  password_sha256.py      
# DESCRIPTION:  This program takes an iput string
#               and prints out the sha256 Hash summary
#				in hexadecimal.
###################################################
# sha256 Password Hex Summary generator

import hashlib
pass_phrase = raw_input("Enter Password >")
result=hashlib.sha256(pass_phrase)
print "The Hexadecimal sha256 Hash summary = ",result.hexdigest()