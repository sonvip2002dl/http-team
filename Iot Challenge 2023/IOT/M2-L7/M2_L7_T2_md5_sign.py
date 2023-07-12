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
# FILE NAME  :  M2_L7_T2_md5_sign.py      
# DESCRIPTION:  This program creates a signiture 
#               that is added to the end of a file.
#               This signiture is created by adding 
#               the pass phrase to the end of the file 
#               as it is hashed using MD5.
###################################################
# md5 signiture generator for a file.

import hashlib
file_name = raw_input("Enter file to add Signature to > ")
try:
	fp=open(file_name,'r')
	data = fp.read()
except:
	print "Cannot open file"
	quit()
pass_phrase = raw_input("Enter Signature > ")
result=hashlib.md5(data+pass_phrase)
print "The Hexadecimal Digital Signature  = ",result.hexdigest()
out_file = raw_input("enter output file [default .sgnd] > ")
if len(out_file) < 1:
	out_file = file_name + ".sgnd"
try:
	fpw = open(out_file,'w')
	fpw.write(data+result.hexdigest())
	fpw.close()
except:
	'print unable to write file: "',out_file,'"'
