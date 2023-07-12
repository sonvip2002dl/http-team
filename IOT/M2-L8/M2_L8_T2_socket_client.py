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
# FILE NAME  :  socket_client.py    
# DESCRIPTION: This program will take send inputted data
# to the port at local host.  
# The socket_server program needs to be running 
# before this program can start.
# #################################################
# The socket server must be running first.
import socket               # Import socket module
import sys

port = 4321 
addr = "127.0.0.1"

try:
     mys = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
except mys.error:
     print("Failed to generate socket")
     sys.exit()

data = raw_input(" Enter Data: ")
try:
   ainfo = socket.getaddrinfo(addr,port)
   mys.connect(ainfo[0][4])
except mys.error:
   print("failed to connect")
   sys.exit()

mys.sendall(data)
mys.close()
