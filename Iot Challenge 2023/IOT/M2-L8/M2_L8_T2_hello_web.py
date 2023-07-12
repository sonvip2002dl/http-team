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
# FILE NAME  :  hello_web.py      
# DESCRIPTION:  This program displays "Hello World"
# as HTML via Python.  This shows request Response Cycle
# #################################################
import socket
import sys
# Define socket to be used for web page  80 is a normal web socket, but may be used for other things in the system.
HOST, PORT = '', 1080

# Define the standard web response for a web page
web_response = """\
HTTP/1.1 200 OK

"""
web_line = '<html><h1>Hello World</h1></html>\n'
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
    request = client_connection.recv(1024)
    print(request)
    try:
       client_connection.sendall(web_response.encode('utf8'))
       client_connection.sendall(web_line.encode('utf8'))
    except:
       client_connection.close()
       print("Web Exception")
    client_connection.close()
