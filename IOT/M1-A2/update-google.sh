#!/bin/bash

#################################################
#         © Keysight Technologies 2019 
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
# FILE NAME  :  update-google.sh      
# DESCRIPTION:  This program sends data to the Cloud
#
# #################################################

curl -L 'https://script.google.com/macros/s/AKfycby152VslPmk4vWsWpuZ4LT4elK4eCsglx9zXSZxfeTYDk_Zwqc/exec?accelX='$1'&accelY='$2'&accelZ='$3
