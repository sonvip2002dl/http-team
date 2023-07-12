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
# FILE NAME  :  aesutil.py      
# DESCRIPTION:  This program provides functions to 
#               encrypt and decrypt the message using 
#               the given key.
###################################################

from Crypto.Cipher import AES
import base64
import hashlib


def Encrypt(message, key):
	hashedKey = hashlib.md5(key).hexdigest()
	iv =  hashlib.md5(key).digest()
	cipher = AES.new(hashedKey, AES.MODE_CFB, iv)
	return base64.b64encode(cipher.encrypt(message))

def Decrypt(ciphertext, key):
	hashedKey = hashlib.md5(key).hexdigest()
	iv =  hashlib.md5(key).digest()
	cipher = AES.new(hashedKey, AES.MODE_CFB, iv)
 	return cipher.decrypt(base64.b64decode(ciphertext))


def EncryptToFile(message, key, filename):
	ciphertext = Encrypt(message, key)
	with open(filename, "w") as write_file:
		write_file.write(ciphertext)

def DecryptFromFile(key, filename):
	with open(filename, "r") as read_file:
		content = read_file.read()
	return Decrypt(content, key)
