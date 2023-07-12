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
# FILE NAME  :  add_user_pass.py      
# DESCRIPTION:  This program takes an input user name
#               and password and second password entry
#		if the passwords match then it adds the 
#               user name + md5_hash(password+username)
#               to the password table
###################################################
# Add user to password file

import usrmgt

existing = True
while (existing  == True):
	user_name = raw_input("Enter user name >")
	existing = usrmgt.IsValidUser(user_name)
	if (existing == True):
		print user_name + " already exist."
		
			
pass_phrase = raw_input("Enter Password >")
pass_phrase2 = raw_input("Re-Enter Password >")
if pass_phrase == pass_phrase2:	
	usrmgt.AddOrUpdateUser(user_name,pass_phrase)
	print user_name + " added."
else:
	print "Passwords do not match"
			



