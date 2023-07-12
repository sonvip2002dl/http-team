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
# FILE NAME  :  session_cookie.py      
# DESCRIPTION:  This program uses session cookies to
# track logins. This calls usrmgt.py to validate login.
# When logged in the page will update every 30 seconds.
###################################################
#

import socket
import sys
import re
import random
from datetime import datetime
import usrmgt
import LCD_Fun as LCD

# Define socket to be used for web page  80 is a normal web socket, but may be used for other things in the system.
HOST, PORT = '', 1080

sessions = {}  # Empty set of logged in sessions

# Define the standard web response for a web page
web_response = """\
HTTP/1.1 200 OK

"""
web_response_cookie = """\
HTTP/1.1 200 OK
Set-Cookie: session="""

web_response_logout = """\
HTTP/1.1 200 OK
Set-Cookie: session=deleted

"""

# Title block for the web page
web_header = '<!DOCTYPE html><html><body><head><meta http-equiv="refresh" content="10"></head><center> <font color="blue"> <h1>Project Board Readings</h1>\n'
login_header = '<!DOCTYPE html><html><body><center> <font color="blue"> <h1>Please Login</h1>\n'

# Function to send the data to the socket.  Will close the connection if there is an error
def send_web(string_data):
        try:
           client_connection.sendall(string_data.encode('utf8'))
        except:
           client_connection.close()
           print("Web Exception")
		
###################################################################################
#
# Main Web Output
#
###################################################################################
def output_web(id):
	global sessions
	if id in sessions:
		if sessions[id]:
			send_web(web_header)
			send_web('<br><h2>Current Time = '+str(datetime.now())+'<br></h2>')
			send_web('<br><form method="post">\n')
			send_web('<input type="submit" name="logout" value="Logout"></form></html>\n')
			return()
	send_web(login_header)
	send_web('<br><form method="post">')
	send_web('User Name:<br><input type="text" name ="uname" ><br>')
	send_web('Password:<br><input type="password" name ="pass" ><br>')
	send_web('<input type="submit" name="login" value="Login"/></form></html>\n')

def auth_user(usr,pwd,id):
	global sessions
	sessions[id]=usrmgt.ValidateUser(usr,pwd)


# Open up a socket server port
listen_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
listen_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
listen_socket.bind((HOST, PORT))
listen_socket.listen(1)
print( 'Serving HTTP on port %s ...' % PORT)
print(web_response)
while True:
# open socket
	client_connection, client_address = listen_socket.accept()
	print 'Client Address = ',client_address[0]
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
        LCD.LCD_clear()
        LCD.LCD_print(client_address[0])
        print 'Client Address = ',client_address[0]

# Look at the various lines coming back.  See if it is a GET or POST  If it is a POST look for our data
	headers_alone = request_dec.split('\r\n')
	session_id = -1
	for line in headers_alone:
		if (line.find('Cookie: session') > -1):
			try:
				session_id = int(re.search(r'\d+',line).group())
				print ("Found cookie session = ",session_id)
			except:
				session_id = -1
				print ("Found cookie could not convert")
	if (headers_alone[0].find('POST') > -1):
		print('Found Post Request')
		if request_dec.find('logout=Logout') > -1:
			print('Logging out User')
			send_web(web_response_logout)
			send_web("<html><body><center> <font color='red'> <h1>Logging Out...Good Bye</h1><br>\n")
			del sessions[session_id]
			#listen_socket.close()
			#sys.exit()
		elif request_dec.find('login=Login') > -1:
			usern = ''
			passw = ''
			for line in headers_alone:
				if(line.find('login=Login') > -1):
					field = line.split('&')
					for fvar in field:
						if fvar.find('pass=') > -1:
							passw = fvar[fvar.find('pass=')+5:]
						if fvar.find('uname=') > -1:
							usern = fvar[fvar.find('uname=')+6:]
					print("User name = "+usern+" Password = "+passw)
			auth_user(usern,passw,session_id)
			send_web(web_response)
			output_web(session_id)
			
		else:
			send_web(web_response)
			send_web("<html><body><center> <font color='green'> <h1>Unknown Post...</h1><br>\n")
			output_web(session_id)
 #For both post and get respond with standard HTTP OK and send Header
	elif (headers_alone[0].find('GET') > -1) | (headers_alone[0].find('POST') > -1):
			print("Got GET request")
			if session_id > 0:	
				send_web(web_response)
			else:
				session_id = random.randrange(10000000)
				send_web(web_response_cookie + str(session_id)+ '\r\n\r\n')
			output_web(session_id)
	else:
		print("got unknown")
		print(request_dec)
	client_connection.close()
        if session_id in sessions:
                if sessions[session_id]:
                     LCD.LCD_print2("Logged in")
                else:
                     LCD.LCD_print2("Not Logged in")
        else:
            LCD.LCD_print2("Not Logged in")
