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
# FILE NAME  :  web_pressure.py      
# DESCRIPTION:  This displays the presure effective
# altitude and temperature from the MPL3115A sensor
# This introduces the "send_web" function
# #################################################
import socket
import sys
import mraa
import time
import Pressure_Fun as Pressure
import mraa_beaglbone_pinmap as pm

# Define socket to be used for web page  80 is a normal web socket, but may be used for other things in the system.
HOST, PORT = '', 1080

# Define the standard web response for a web page
web_response = """\
HTTP/1.1 200 OK

"""
web_header = '<!DOCTYPE html><html><body><center> <font color="blue"> <h1>Project Board Readings</h1>\n'

# MPL3115A2 Address mpl_addr
mpl_addr = 0x60

# Function to send the data to the socket.  Will close the connection if there is an error
def send_web(string_data):
        try:
           client_connection.sendall(string_data.encode('utf8'))
        except:
           client_connection.close()
           print("Web Exception")



Pressure.Pressure_init()

# Open up a socket server port
listen_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
listen_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
listen_socket.bind((HOST, PORT))
listen_socket.listen(1)
print( 'Serving HTTP on port %s ...' % PORT)
while True:
	# open socket
    client_connection, client_address = listen_socket.accept()
# wait for response
    try:
         request = client_connection.recv(1024)
         print(request)
    except:
         request = "Null"
    try:
        send_web(web_response)
        send_web(web_header)
        out = Pressure.Pressure_read() 
        send_web("<h3>MPL3115A2 Readings<br>")
        send_web("Pressure = "+str(out[0]) + "mb  Altitude = "+str(out[1]) + " Feet<br>")
        send_web("Temperature ="+str(out[2]) + "'C  Temperature = "+str(out[3]) + "'F<br></h3></html>\n\r")

    except:
       client_connection.close()
       print("Web Exception")
    client_connection.close()
