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
# FILE NAME  :  web.py      
# DESCRIPTION:  This program uses the pressure sensor,
# ADC with TMP36, Relay on GP2 and the buttons.  This
# calls usrmgt.py to validate login.
# This uses LCD_Fun, Pressure_Fun, and adcPythonC
# to capture the sensor data.
# When logged in the page will update every 30 seconds.
###################################################
#
import socket
import sys
import urllib
import mraa
import time
import re
import random
from datetime import datetime
import usrmgt
import LCD_Fun as LCD
from adcPythonC import *
import Pressure_Fun as Pressure
import mraa_beaglbone_pinmap as pm

# Define socket to be used for web page  80 is a normal web socket, but may be used for other things in the system.
HOST, PORT = '', 1080

# Define the standard web response for a web page
# MPL3115A2 Address mpl_addr
mpl_addr = 0x60
update = 3000
adc_channel = 7 # Input pin for ADC Channel
buttons = []
button_pins = [pm.B1,pm.B2pm.B3,pm.B4]
for button in button_pins:
    buttons.append(mraa.Gpio(button))
relay0_state = 0
relay_pins = [pm.GP2,pm.GP3]
relays=[]
print(str(mraa.DIR_OUT))
for r in relay_pins:
    relays.append(mraa.Gpio(r))
for button in buttons:
    button.dir(mraa.DIR_IN)
for relay in relays:
    time.sleep(1)   # BB requires 1 second pause befor changing direction!
    r = relay.dir(mraa.DIR_OUT)
    print("status ="+str(r))
    relay.write(0)

# Open I2C bus
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
web_header = '<html><body><head><meta http-equiv="refresh" content="10"></head><center> <font color="blue"> <h1>Project Board Readings</h1>\n'
login_header = '<html><body><center> <font color="blue"> <h1>Please Login</h1>\n'

# Function to send the data to the socket.  Will close the connection if there is an error
def send_web(string_data):
        try:
           client_connection.sendall(string_data.encode('utf8'))
        except:
           client_connection.close()
           print("Web Exception")

def output_web(id):
	global sessions
        print("Output Web")
	if id in sessions:
		if sessions[id]:
			main_web()
			return()
	send_web(login_header)
	send_web('<br><form method="post">')
	send_web('User Name:<br><input type="text" name ="uname" ><br>')
	send_web('Password:<br><input type="password" name ="pass" ><br>')
	send_web('<input type="submit" name="login" value="Login"/></form></p>\n')
        print("Sent Login Form")

def main_web():
    out = Pressure.Pressure_read() 
    tempa = round(100*(adcRead(adc_channel) * 3.3/4095.0 -0.5),2)
    send_web(web_header)
    send_web("<h3>MPL3115A2 Readings<br>")
    send_web("Pressure = "+str(out[0]) + "mb  Altitude = "+str(out[1]) + " Feet<br>")
    send_web("Temperature ="+str(out[2]) + "'C  Temperature = "+str(out[3]) + "'F<br></h3>")
    send_web("<h2>TMP36 Reading = "+str(tempa)+"'C  "+str(round(tempa*9/5.0 +32,2))+"'F </h2>")  
    send_web("<font color='black'><h2> Relay Status = ")
    if(relay0_state == 0 ):
        send_web("<font color='red'> Off<br><font color='black'>")
    if(relay0_state == 1 ):
        send_web("<font color='green'> On<br><font color='black'>")
    send_web("Button Status</h2><h3>")
    but_number = 1
    for button in buttons:
        if(button.read() == 0):
            send_web(" Button "+str(but_number)+":<font color='green'> On  <font color='black'>")
        else:
            send_web(" Button "+str(but_number)+":<font color='blue'> Off  <font color='black'>")
        but_number += 1
    send_web('<br><br>Set Relay<br><form method="post">\n')
    send_web('<input type="submit" name="On" value="on">')
    send_web('<input type="submit" name="Off" value="off"></p>\n')
    send_web('<input type="submit" name="logout" value="Logout"></form></p>\n')
	
	
def auth_user(usr,pwd,id):
	global sessions
	sessions[id]=usrmgt.ValidateUser(usr,pwd)
	

LCD.LCD_init()
Pressure.Pressure_init()
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
	# Find out what type of request it is POST = data coming back.  GET is a regular request.
    for line in headers_alone:  # Check for forwared address...
	if (line.find('X-Forwarded-For') > -1):
		try:
			aa=re.findall( r'[0-9]+(?:\.[0-9]+){3}',line)
			client_address[0]=aa.decode()
		except:
			print("Cannot Decode Address")
    LCD.LCD_clear()
    LCD.LCD_print(client_address[0])
    print 'Client Address = ',client_address[0]
    session_id = -1
    for line in headers_alone:
	if (line.find('Cookie: session') > -1):
		try:
			session_id = int(re.search(r'\d+',line).group())
			print ("Found cookie session = "+str(session_id))
		except:
			session_id = -1
			print ("Found cookie could not convert")
    if session_id in sessions:
          if sessions[session_id]:
              LCD.LCD_print2("Logged in")
          else:
              LCD.LCD_print2("Not Logged in")
    else:
          LCD.LCD_print2("Not Logged in")
    if (headers_alone[0].find('POST') > -1):
	print('Found Post Request')
	if request_dec.find('logout=Logout') > -1:
		print('Logging out Session id '+ str(session_id))
		send_web(web_response_logout)
		send_web("<html><body><center> <font color='red'> <h1>Logging Out...Good Bye</h1><br>\n")
		#change session id to logged out.
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
						passw = urllib.unquote_plus(fvar[fvar.find('pass=')+5:])
					if fvar.find('uname=') > -1:
						usern = fvar[fvar.find('uname=')+6:]
				# print("User name = ",usern," Password = ",passw)
		auth_user(usern,passw,session_id)
		send_web(web_response)
		output_web(session_id)
	elif (session_id in sessions.keys()):
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
			session_id = random.randrange(1000000)
			send_web(web_response_cookie + str(session_id)+ '\r\n\r\n')

		output_web(session_id)
    else:
	print("got unknown")
	print(request_dec)
    client_connection.close()
	
	
