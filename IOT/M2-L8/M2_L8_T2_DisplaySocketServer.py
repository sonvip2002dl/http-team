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
# FILE NAME  :  DisplaySocketServer.py    
# DESCRIPTION: This program will take in data presented
# to the port and print it out as well as display the
# data and the source address on the LCD.  Note this program 
# has provisions to not crash.  This also call LCD_Fun
# This program needs to be running before any socket_clients
# start.
# #################################################

import socket               # Import socket module
import sys
import mraa
import LCD_Fun as LCD		# Import LCD Display Functions

LCD.LCD_init()   # Initialize the LCD

try:
     mys = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  
except:
     print("Failed to generate socket")
     sys.exit()
port = 4321 
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
	LCD.LCD_clear();
	LCD.LCD_print(addr[0])
        print 'Got connection from', addr
        try:
           data = conn.recv(256)
           print data
           dat_len = len(data)
           print"Data Length = ", dat_len
           if dat_len < 20:
               LCD.LCD_print2(data)
           else:
               LCD.LCD_print2(data[:19])
        except:
            print "No Packets"
        conn.close()                # Close the connection
