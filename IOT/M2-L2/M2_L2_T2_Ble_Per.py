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
# FILE NAME  :  M2_L2_T2_Ble_Per.py      
# DESCRIPTION:  This program scan BLE packets and 
#               print out average RSSI and PER. 
# #################################################

import os
import csv
import sys
import subprocess

# Checking the number of argument
if len(sys.argv) <> 3:
    print "Not enough arguments"
else:

    #Get argument value
    Scan_Time = sys.argv[1]
    MAC_ADD = sys.argv[2]

    #r+ because the file is already there, w without the file
    myfile = open("scan.csv", "w")

    #Reset Bluetooth interface, hci0
    os.system("sudo hciconfig hci0 down")
    os.system("sudo hciconfig hci0 up")

    #Scan for bluetooth devices
    hciLescan = subprocess.Popen(["sudo timeout %s hcitool lescan --duplicates" %(Scan_Time)], stdout=subprocess.PIPE, shell=True)
    dev = subprocess.Popen(["sudo timeout %s btmon" %(Scan_Time)], stdout=subprocess.PIPE, shell=True)
    # dev = subprocess.Popen(["sudo timeout %s hcitool lescan --duplicates & sudo timeout %s btmon" %(Scan_Time,Scan_Time)], stdout=subprocess.PIPE, shell=True)
    (device, err) = dev.communicate()

    #Print bluetooth devices
    # print device

    #Write the hcitool lescan output to a file
    myfile.write(device)

    #Close the file
    myfile.close()

    Data1 = []
    Adv_array = []
    Rssi_array = []
    RSSI_value = []
    ADV_IND_array = []
    SCAN_RSP_array = []
    Time_ADV_array = []
    Time_RES_array = []
    Time_ADV_value = []
    Time_RES_value = []

    #Get all line with delimiter
    with open("scan.csv") as f:
        reader = csv.reader(f,delimiter=' ')
        for line in reader:
            Data1.append(line)

        # Get the line of Mac address    
        for line, row in enumerate(Data1):
            try:
                first_column1 = row[9]
            except IndexError:
                    continue
            else:
                if first_column1.strip()==MAC_ADD:
                    Adv_array.append(line - 2)
                    
        # Device checking 
        if len(Adv_array) <> 0:

            # Acquire the details of AVD_IND and SCAN_RES
            for line, row in enumerate(Data1):
                for line_adv, row2 in enumerate(Adv_array):
                    if line == row2:
                        if Data1[line][len(Data1[line])-2] == 'ADV_IND':
                            ADV_IND_array.append(Data1[line][len(Data1[line])-2])
                            Rssi_array.append(line + 11)
                            RSSI_value.append(float(Data1[line + 11][len(Data1[line + 11])-3]))
                            Time_ADV_array.append(line - 3)
                            Time_ADV_value.append(float(Data1[line - 3][len(Data1[line - 3])-1]))
                        if Data1[line][len(Data1[line])-2] == 'SCAN_RSP':
                            SCAN_RSP_array.append(Data1[line][len(Data1[line])-2])
                            Rssi_array.append(line + 7)
                            RSSI_value.append(float(Data1[line + 7][len(Data1[line + 7])-3]))
                            Time_RES_array.append(line - 3)
                            Time_RES_value.append(float(Data1[line - 3][len(Data1[line - 3])-1]))

            # Calculation parts
            PER = (float(len(ADV_IND_array) - len(SCAN_RSP_array))/len(ADV_IND_array))*100
            Avr_RSSI = float(sum(RSSI_value)/len(RSSI_value))

            Average_AdvInterval = (Time_ADV_value[len(Time_ADV_value)-1] - Time_ADV_value[0])/len(ADV_IND_array)
            Average_ResInterval = (Time_RES_value[len(Time_RES_value)-1] - Time_RES_value[0])/len(SCAN_RSP_array)
            Adv_AvgPacPerSec = 1/Average_AdvInterval
            Res_AvgPacPerSec = 1/Average_ResInterval

            print "AdvCount: ", len(ADV_IND_array), "\tPER: %.2f"%(PER), "%", "\tAvg RSSI: %.2f"%(Avr_RSSI)," dBm","\tAvgAdvInterval: %.6f s" % (Average_AdvInterval), "\tAvgPkt/sec : %.6f" %(Adv_AvgPacPerSec)
            print "ResCount: ", len(SCAN_RSP_array), "\t\t\t\t\t\tAvgResInterval: %.6f s" %(Average_ResInterval), "\tAvgPkt/sec : %.6f" %(Res_AvgPacPerSec)
        
        else:
            print "No device found"

