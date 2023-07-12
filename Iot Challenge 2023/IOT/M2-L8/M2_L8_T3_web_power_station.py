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
# FILE NAME  :  web_power_station.py      
# DESCRIPTION:  This program uses the pressure sensor,
# ADC with TMP36, Relay on GP2 and the buttons.  This
# calls usrmgt.py to validate login.
# This uses LCD_Fun, Pressure_KPa, and adcPythonC
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
import Pressure_KPa as Pressure
import mraa_beaglbone_pinmap as pm

# Define socket to be used for web page  80 is a normal web socket, but may be used for other things in the system.
HOST, PORT = '', 1080

# Define the standard web response for a web page
# MPL3115A2 Address mpl_addr
mpl_addr = 0x60
update = 3000
adc_channel = 7 # Input pin for ADC Channel
buttons = []
button_pins = [pm.B1,pm.B2,pm.B3,pm.B4]
for button in button_pins:
    buttons.append(mraa.Gpio(button))

relay_pins = [pm.GP2,pm.GP3]
relays=[]
for r in relay_pins:
    relays.append(mraa.Gpio(r))
for button in buttons:
    button.dir(mraa.DIR_IN)
for relay in relays:
    time.sleep(1)   # BB requires 1 second pause befor changing direction!
    r = relay.dir(mraa.DIR_OUT)
    while r != 0:
         print("status ="+str(r))
         time.sleep(1)
         r = relay.dir(mraa.DIR_OUT)
    relay.write(0)

# Open I2C bus
sessions = {}  # Empty set of logged in sessions

# Define the standard web response for a web page Note the Triple " makes the new lines part of the string
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

# Title block for the web page Including the DOM doc type and 10 second refresh
web_header = '<!DOCTYPE html><html><head><meta http-equiv="refresh" content="10"></head><body><center> <font color="blue"> <h1>U3810A Power Station</h1>\n'
# Login Header for web page. Including the DOM doc type
login_header = '<!DOCTYPE html><html><body><center> <font color="blue"> <h1>Please Login</h1>\n'

# Function to send the data to the socket.  Will close the connection if there is an error
def send_web(string_data):
    global client_connection
    try:
       client_connection.sendall(string_data.encode('utf8'))
    except:
       client_connection.close()
       print("Web Exception")

# Function to decide and send login webpage or main webpage.  Checks the session id
def output_web(id):
	global sessions
        print("Output Web")
	if id in sessions:
		if sessions[id]:
			main_web()
			return()
	send_web(login_header)
	send_web('<br><form method="post">\n')
	send_web('User Name:<br><input type="text" name ="uname" ><br>\n')
	send_web('Password:<br><input type="password" name ="pass" ><br>\n')
	send_web('<input type="submit" name="login" value="Login"></form><br>\n</body></html>\n\r')
        print("Sent Login Form")

# main web page this sends the data to the web
def main_web():
    global BoilerFlame,Turbine
    out = Pressure.Pressure_read() 
    tempa = round(100*(adcRead(adc_channel) * 3.3/4095.0 -0.5),2)
    send_web(web_header)
    send_web("<h3>Power Station Status<br>")
    send_web("Boiler Pressure = "+str(round(out[0]+BoilerAddPressure)) + "kPa <br>")
    send_web("Boiler Temperature ="+str(round(out[2]+BoilerAddTemp)) + "'C<br>")
    send_web("Outlet Temperature = "+str(OutletTemp)+"'C </h3>\n")  
    send_web("<font color='black'><h2>Flame Status: \n")
    if(BoilerFlame):
        send_web("<font color='red'> Boiler Flame On<br><font color='black'>")
    else:
        send_web("<font color='gray'> Boiler Flame Off<br><font color='black'>\n")
    send_web("<font color='black'> Turbine Status: ")
    if(Turbine):
        send_web("<font color='green'> Turbine Engaged<br><font color='black'>")
    else:
        send_web("<font color='gray'> Turbine Off<br><font color='black'>")
    send_web("Power Out = "+str(round(PowerOut))+" Watts</h2>\n")
    send_web("<h3><br>Button Status<br/>")
    but_number = 1
    for button in buttons:
        if(button.read() == 0):
            send_web(" Button "+str(but_number)+":<font color='green'> On  <font color='black'>")
###################################################
# Local Override for boiler and turbine
###################################################
	    if but_number == 1:
		BoilerFlame = True
	    if but_number == 2:
		BoilerFlame = False
	    if but_number == 3:
		Turbine = True
	    if but_number == 4:
		Turbine = False
        else:
            send_web(" Button "+str(but_number)+":<font color='blue'> Off  <font color='black'>")
        but_number += 1
    send_web('\n<br><br>Boiler Turbine<br/><form method="post">\n')
    send_web('<input type="submit" name="On" value="on">')
    send_web('<input type="submit" name="Off" value="off">   \n')
    send_web('<input type="submit" name="Ton" value="on">')
    send_web('<input type="submit" name="Toff" value="off"><br/>\n')
    send_web('<input type="submit" name="logout" value="Logout"></form></h3>\n</body></html>\n\r')
	
# Call the user management function to validate login	
def auth_user(usr,pwd,id):
	global sessions
	sessions[id]=usrmgt.ValidateUser(usr,pwd)


#############################################################
#
# Main Program starts here
#
#############################################################
	
# Intitalize sensors and Power Station State variables
LCD.LCD_init()
if not Pressure.Pressure_init():
	print("Pressure Sensor not present exiting program")
	sys.exit()

PresSensorReading = Pressure.Pressure_read() 
VaporTempFactor = 0.5    # Factor for boining point versus Temperature    
AtmosPressure = PresSensorReading[0]
BoilerPressure = PresSensorReading[0]
BoilerTemp = PresSensorReading[2] 
BoilerAddTemp = 0.0
BoilerAddPressure = 0.0
TempReading = round(100*(adcRead(adc_channel) * 3.3/4095.0 -0.5),2)
OutletInitialTemp = TempReading
OutletTemp = TempReading+10*(TempReading-OutletInitialTemp)
BoilerFlame = False
Turbine = False
TBoil = 100	# Boiling point at 100kPa
MaxTemp = 300   # Maximum Boiler Termperature
PowerOut = 0

# Open up a socket server port
listen_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
listen_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
listen_socket.bind((HOST, PORT))
listen_socket.listen(1)
print( 'Serving HTTP on port %s ...' % PORT)

# Main Loop
while True:
    #######   Power station state machine
    PresSensorReading = Pressure.Pressure_read() 
    TempReading = round(100*(adcRead(adc_channel) * 3.3/4095.0 -0.5),2)
    OutletTemp = TempReading+10*(TempReading-OutletInitialTemp)
    BoilerTemp = PresSensorReading[2] + BoilerAddTemp
    if(BoilerTemp) > MaxTemp:
	BoilerFlame = False

    if(BoilerFlame):  # Note Low = on and High = off
        relays[0].write(0)
	BoilerAddTemp += 3.4
    else:
        relays[0].write(1)
        if (BoilerAddTemp > 0):
            BoilerAddTemp -= 0.2
    if(Turbine):  # Note Low = on and High = off
        relays[1].write(0)
        PressureDiff = BoilerPressure - AtmosPressure 
        PowerOut = PressureDiff * (BoilerTemp - OutletTemp)
        BoilerAddPressure -= 1.2*PressureDiff/100
        BoilerAddTemp -=  1.4*PressureDiff/100
        if BoilerAddTemp < 0:
            BoilerAddTemp = 0
    else:
        relays[1].write(1)
        PowerOut = 0
    TBoil = 50 + BoilerPressure*VaporTempFactor
    if (PresSensorReading[2] + BoilerAddTemp) > TBoil:    # Add pressure if boiling
          BoilerAddPressure += 3.4
    elif BoilerAddPressure > 0.5:                         #Decrease pressure if below boiling and postivi pressure
          BoilerAddPressure -= 0.5

    BoilerPressure = PresSensorReading[0] + BoilerAddPressure
	##### End of Power station
	
# open socket  **** Note the program will idle here until connection is made
    client_connection, client_address = listen_socket.accept()
    list(client_address)
# Get data from the user
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

    cl_addr = str(client_address[0])
    for line in headers_alone:  # Check for forwared address...
	if (line.find('X-Forwarded-For') > -1):
		try:
			aa=re.findall( r'[0-9]+(?:\.[0-9]+){3}',line)
			addrdecode = str(aa[0].decode())
			cl_addr = addrdecode
		except:
			print("Cannot Decode Address")
			cl_addr=str(client_address[0])
		
    LCD.LCD_clear()
    LCD.LCD_print(cl_addr)
    print 'Client Address = ',cl_addr
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
# Look for data in the request
	  if request_dec.find('logout=Logout') > -1:
		  print('Logging out Session id '+ str(session_id))
		  send_web(web_response_logout)
		  send_web("<!DOCTYPE html><html><body><center> <font color='red'> <h1>Logging Out...Good Bye</h1></body></html>\n\r")
		  #change session id to logged out.
		  del sessions[session_id]

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
# if the session is logged in then it can turn on or off controls.		
	  elif (session_id in sessions.keys()):
	       if (sessions[session_id]):
		  out_test = request_dec.find('On=on')
		  if out_test > 1:
			  BoilerFlame = True
			  print("got On request")
			  relays[0].write(0)
		  out_test = request_dec.find('Off=off')
		  if out_test > 1:
			  BoilerFlame = False
			  print("Got off request")
			  relays[0].write(1)
		  out_test = request_dec.find('Ton=on')
		  if out_test > 1:
			  Turbine = True
			  print("got On request")
			  relays[1].write(0)
		  out_test = request_dec.find('Toff=off')
		  if out_test > 1:
			  Turbine = False
			  print("Got off request")
			  relays[1].write(1)

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
	# Pnirt out the request
	print(request_dec)
	 # Closing the connection completes the webpage ouput in Request response cycle
    client_connection.close()
	
	
