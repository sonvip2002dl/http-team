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
# FILE NAME  :  Pressure_Fun.py      
# DESCRIPTION:  This program gets pressure, altitude 
#               and Temperature using mraa library
# #################################################

import mraa
import time
import mraa_beaglbone_pinmap as pm
# MPL3115A2 Address mpl_addr
mpl_addr = 0x60
update = 3000
# Get I2C bus
bus = mraa.I2c(pm.I2CS_BUS,True)
status = bus.address(mpl_addr)
def Pressure_read():
     # MPL3115A2 address, mpl_addr(0x60)
     # Select control register, 0x26(38)
     #		0xB9(185)	Active mode, OSR = 128, Altimeter mode
     bus.writeReg(0x26, 0xB9)
     # MPL3115A2 address, mpl_addr(96)
     # Select data configuration register, 0x13(19)
     #		0x07(07)	Data ready event enabled for altitude, pressure, temperature
     bus.writeReg(0x13, 0x07)
     # MPL3115A2 address, mpl_addr(96)
     # Select control register, 0x26(38)
     #		0x39(57)	Active mode, OSR = 128, Barometer mode
     bus.writeReg(0x26, 0x39)
     time.sleep(2)

     # MPL3115A2 address, mpl_addr(96)
     # Read data back from 0x00(00), 4 bytes
     # status, pres MSB1, pres MSB, pres LSB
     data = bus.read(6)

     # Convert the data to 20-bits
     pres = ((data[1] * 65536) + (data[2] * 256) + (data[3] & 0xF0)) / 16
     pressure = (pres / 4.0) / 1000.0
     temp = ((data[4] * 256) + (data[5] & 0xF0)) / 16
     cTemp = temp / 16.0
     fTemp = cTemp * 1.8 + 32
     altitude = 0.0
     # Output data to screen
     pressure = round(pressure,1)
     altitude = round(altitude,1)
     cTemp = round(cTemp,1)
     fTemp = round(fTemp,1)
     return([pressure, altitude, cTemp, fTemp])
def Pressure_init():
	# Need to initialize Pressure sensor so first readings are non zero.
     status1 = bus.writeReg(0x26, 0x39)
     status2 = bus.writeReg(0x13, 0x07)
     if (status1 != 0) | (status2 != 0):
         print "Pressure Sensor Error, Please check for 0x60 on i2cdetect -r -y 1"
         return(False)
     else:
         return(True)
     
