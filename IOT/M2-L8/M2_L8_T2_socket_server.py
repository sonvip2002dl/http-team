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
# FILE NAME  :  socket_server.py    
# DESCRIPTION: This program will take in data presented
# to the port and print it out.  Note this program will crash
# if not presented with the correct data.
# This program needs to be running before any socket_clients
# start.
# #################################################
import socket               # Import socket module
import sys

port = 4321 

try:
     mys = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  
except:
     print("Failed to generate socket")
     sys.exit()

try:
   mys.bind(("", port))        # Bind to the port
except mys.error:
   print"failed to connect"
   sys.exit()

while True:
     print "listing port 4321"
     mys.listen(5)                 # Now wait for connection
     while True:
        conn, addr = mys.accept()
        print 'Got connection from', addr
        data = conn.recv(200)
        print data
        conn.close()                # Close the connection
