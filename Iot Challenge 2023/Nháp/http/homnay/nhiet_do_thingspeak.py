import Adafruit_BBIO.ADC as ADC  
import time  
import httplib, urllib  
sensor_pin = 'P9_40'  
ADC.setup()  
print('Reading\t\tVolts')  
while True:  
     reading = ADC.read(sensor_pin)  
     volts = reading * 1.800  
     params = urllib.urlencode({'field1': volts,'key':'229RF49BOZ5B8T7C'})  
     headers = {"Content-type": "application/x-www-form-urlencoded","Accept":"text/plain"}  
     conn = httplib.HTTPConnection("api.thingspeak.com:80")  
     conn.request("POST", "/update", params, headers)  
     print('%f\t%f' % (reading, volts))  
     res = conn.getresponse()  
     print res.status, res.reason  
     time.sleep(16)  