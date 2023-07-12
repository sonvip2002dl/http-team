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
# FILE NAME  :  M2_L7_T2_md5_check.py      
# DESCRIPTION:  This program extracts the digital 
# 		signiture from the end of a document
#   		then the signiture is compaired against
#               the document with the pass phrase.
#               if any bit of the document was changed
#               the signiture check will fail.
###################################################
# MD5 signiture checker for a file
# 

import hashlib
hash_len = 32   # Length of hex hash summary
file_name = raw_input("Enter file to check Signature > ")
try:
	fp=open(file_name,'r')
	data = fp.read()
except:
	print "Cannot open file"
	quit()
pass_phrase = raw_input("Enter Signature to check against > ")
doc_len = len(data)
if doc_len < hash_len:  #Check to make sure the file is bigger than the hash length 
	print "Document file does not have a signature or is corrupted"
	quit()
doc_hashdigest = data[doc_len-hash_len:]
print "Document Hash Digest = ",doc_hashdigest
result=hashlib.md5(data[:doc_len-hash_len]+pass_phrase)
print "The Hexadecimal Digital Signature  = ",result.hexdigest()
if doc_hashdigest == result.hexdigest():
	print "Digital signature is OK"
        out_file = raw_input("enter output file [No output] >")
        if len(out_file) > 1:
            try:
                fpw = open(out_file,'w')
                fpw.write(data[:doc_len-hash_len])  # Remove the Hash from the end of the file
                fpw.close()
            except:
                'print unable to write file: "',out_file,'"'


else:
	print "The digtal signature does not match!"
