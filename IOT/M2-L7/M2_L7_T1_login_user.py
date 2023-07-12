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
# FILE NAME  :  M2_L7_T1_login_user.py      
# DESCRIPTION:  This program takes an input user name
#               and password and validates the login
#		against saved user password file.
###################################################
# Check a user name to a hashed password in a file.

import usrmgt
logged_in = False
while not logged_in:
	user_name = raw_input("Enter user name >")
	pass_phrase = raw_input("Enter Password >")
	result=usrmgt.ValidateUser(user_name,pass_phrase)
	if (result == True):
		print "Logged in"
		logged_in = True
	else:
		print "User Name and Password do not match"
