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
# FILE NAME  :  M1_L7_Pressure_GS.py      
# DESCRIPTION:  This program gets pressure, altitude 
#               and Temperature using mraa library
#               and posts on Google sheets
# #################################################

import time
import datetime
import M1_L7_LCD_Fun as LCD
import M1_L7_Pressure_Fun as Pres
import gspread
from oauth2client.service_account import ServiceAccountCredentials

LCD.LCD_init()   # Initialize the LCD
if( not Pres.Pressure_init()):
    print "Pressure Sensor initialization error.  Exiting program"
    exit()
LCD.LCD_clear()

# Replace the documentKey with your google spreadsheet document key.
documentKey = "<<Your Google Sheets Document Key >>";

# Replace the jsonFile name with your .json file name.
jsonFile = "MyProject.json"

scope = ['https://spreadsheets.google.com/feeds']
creds = ServiceAccountCredentials.from_json_keyfile_name(jsonFile, scope)
client = gspread.authorize(creds)
sheet = client.open_by_key(documentKey).sheet1

while True:
        Pressure_Data = Pres.Pressure_read()
        LCD.LCD_print("Pressure  Temp")
        pressure = str(Pressure_Data[0])
        temp = str(Pressure_Data[2])
        if (1000.00 <= Pressure_Data[0]):
            LCD.LCD_print2(pressure + "mB  " + temp+"C")
        else:
            LCD.LCD_print2(pressure + " mB  " + temp+"C")
	now = datetime.datetime.now()
        try:  # Note Google Sheets need to re-authenicate every hour.
	    sheet.append_row([now.strftime("%Y-%m-%d %H:%M"),temp,pressure])		
        except:
            print("Reauthinicating")
            client = gspread.authorize(creds)
            sheet = client.open_by_key(documentKey).sheet1

        print(" posted data "+now.strftime("%y-%m-%d %H:%M"))
        time.sleep(300)

