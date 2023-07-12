import os
import pexpect
import sys
import time

bluetooth_adr = "24:71:89:07:84:03" # Change this

# Connect to SensorTag
tool = pexpect.spawn('gatttool -b ' + bluetooth_adr + ' --interactive')
tool.expect('\[LE\]>')
print "Connecting to SensorTag... You might need to press the side button..."
tool.sendline('connect')
# Check connection
tool.expect('Connection successful.*\[LE\]>')
# Enable temperature sensor
tool.sendline('char-write-req 0x2F 01')
tool.expect('\[LE\]>')
time.sleep(1)

while True:
	# Read temperature data
	tool.sendline('char-read-hnd 0x2C')
	tool.expect('descriptor: .*') 
	rval = tool.after.split()
	data = rval[1]+rval[2]+' '+rval[3]+rval[4]
	cmd = "./print2lcd "+data
	# Display data on LCD
	os.system(cmd)
	time.sleep(1)
