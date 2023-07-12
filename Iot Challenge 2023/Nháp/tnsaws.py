import time
import paho.mqtt.client as mqtt
import ssl
import thread

def on_connect(client, userdata, flags, rc):
    print "Connected with result code "+str(rc)

client = mqtt.Client()


client.tls_set(
    ca_certs='./rootCA.pem',
    certfile='./d9e24d88981687a2177e0c939b27b0110fd41524c9d1ce05842867c683f1be1c-certificate.pem.crt',
    keyfile='./d9e24d88981687a2177e0c939b27b0110fd41524c9d1ce05842867c683f1be1c-private.pem.key',
    tls_version=ssl.PROTOCOL_SSLv23
)

client.tls_insecure_set(True)


client.username_pw_set("your-username", "your-password") 
client.connect("a1t91yaud0zofo-ats.iot.us-east-2.amazonaws.com", 8883, 60) 

def intrusionDetector(dummy):
    while True:
        print "TN-S"
        client.publish("tn-s", payload="Luong mua:...mm \r\n Muc nuoc:...m \r\n Muc canh bao:    \r\n", qos=0, retain=False)
        time.sleep(5)


thread.start_new_thread(intrusionDetector, (None,))

client.loop_forever()
'''
import serial

#UART
ser = serial.Serial('/dev/ttyS1', 9600)

while True:
    if ser.in_waiting > 0:
        data = ser.readline().decode().strip()
        print("Received: {}".format(data))
        #upload to aws
        client.publish("tn-s", payload=data, qos=0, retain=False)
'''