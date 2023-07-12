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
# FILE NAME  :  web_relay.py      
# DESCRIPTION:  This program will display the relay
# status and have buttons to turn turn the relay on
# or off.  This makes use of post data sent from the
# web request.
###################################################
#
import socket
import sys
import urllib
import mraa
import time
import mraa_beaglbone_pinmap as pm

# Define socket to be used for web page  80 is a normal web socket, but may be used for other things in the system.
HOST, PORT = '', 1080

# Define the standard web response for a web page
# MPL3115A2 Address mpl_addr
relay0_state = 0
relay_pins = [pm.GP2,pm.GP3]
relays=[]
for r in relay_pins:
    relays.append(mraa.Gpio(r))
for relay in relays:
    time.sleep(1)   # BB requires 1 second pause befor changing direction!
    r = relay.dir(mraa.DIR_OUT)
    relay.write(0)

# Define the standard web response for a web page
web_response = """\
HTTP/1.1 200 OK

"""

# Title block for the web page
web_header = '<html><body><center><font color="blue"> <h1>Relay Page</h1>\n'

# Function to send the data to the socket.  Will close the connection if there is an error
def send_web(string_data):
        try:
           client_connection.sendall(string_data.encode('utf8'))
        except:
           client_connection.close()
           print("Web Exception")

# Open up a socket server port
listen_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
listen_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
listen_socket.bind((HOST, PORT))
listen_socket.listen(1)
print( 'Serving HTTP on port %s ...' % PORT)

while True:
    if(relay0_state == 0):  # Note Low = on and High = off
        relays[0].write(1)
    if(relay0_state  == 1 ):
        relays[0].write(0)
# open socket
    client_connection, client_address = listen_socket.accept()
# wait for response
    try:
        request = client_connection.recv(1024)
    except:
        request = "Null"
    print(request)
# Change response from bytes to regular text
    try:
        request_dec = request.decode()
    except:
        request_dec = "Bad Decode"
# Look at the various lines coming back.  See if it is a GET or POST  If it is a POST look for our data
    headers_alone = request_dec.split('\r\n')
	# Find out what type of request it is POST = data coming back.  GET is a regular request.
    if (headers_alone[0].find('POST') > -1):
	out_test = request_dec.find('On=on')
	if out_test > 1:
		relay0_state = 1
		print("got On request")
                relays[0].write(0)
	out_test = request_dec.find('Off=off')
	if out_test > 1:
		relay0_state = 0
		print("Got off request")
                relays[0].write(1)
 #For both post and get respond with standard HTTP OK and send Header
    if (headers_alone[0].find('GET') > -1) | (headers_alone[0].find('POST') > -1):
	send_web(web_response)
	send_web(web_header)
        send_web("<font color='black'><h2> Relay Status = ")
        if(relay0_state == 0 ):
              send_web("<font color='red'> Off<br><font color='black'>")
        if(relay0_state == 1 ):
             send_web("<font color='green'> On<br><font color='black'>")
        send_web('<h3>Set Relay<br><form method="post">\n')
        send_web('<input type="submit" name="On" value="on">')
        send_web('<input type="submit" name="Off" value="off"></p></form></h3>\n')
        
    else:
	print("got unknown")
	print(request_dec)
    client_connection.close()
	
	
