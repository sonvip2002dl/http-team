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
# FILE NAME  :  ADC_Python.py      
# DESCRIPTION:  This program calls the C function
#               from adcPythonC shared object.
###################################################

from adcPythonC import *
for i in range(8):
    print "Result from ADC Channel ",i," = ", adcRead(i)
