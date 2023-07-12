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
# FILE NAME  :  usrmgt.py      
# DESCRIPTION:  This program allows client to Add, Update,
#               Delete or Validate a user against given
#		user name and password. The user information
#               will be saved into a json file with the hashed
#               password.
###################################################

import json
import os
import hashlib

users = {}
userfile = 'users.json'
salt = "x&4ewXeq"

exist = os.path.exists(userfile)

if (exist == True) :
	try:
		with open(userfile, 'r') as read_file:
			users=json.load(read_file)

	except:
		print "Error loading " + userfile + " file."

def AddOrUpdateUser(username, pwd):
	hashpwd=hashlib.md5(pwd+username+salt).hexdigest()
	users[username]=hashpwd
	Save()

def DeleteUser(username):
	users.pop(username)
	Save()

def ValidateUser(username, pwd):
	if (username not in users):
		print "'"+ username + "' not found."
		return False
        hashpwd=hashlib.md5(pwd+username+salt).hexdigest()
	match = hashpwd == users[username]	
	if (match == False):
		print "User authentication failed"
	return match	

def IsValidUser(username):
	return username in users

def Save():
	with open(userfile, 'w') as write_file:
		json.dump(users, write_file, indent=4)
	


	