# -*- coding: utf-8 -*-
import requests
import time
import Adafruit_BBIO.GPIO as GPIO

url = "https://api.thingspeak.com/channels/1956999/field/1/last"

# C?u h?nh ch�n P9_41 l� �?u ra
GPIO.setup("P9_41", GPIO.OUT)

while True:
    # G?i y�u c?u GET �?n URL v� l�u tr? ph?n h?i trong bi?n response
    response = requests.get(url)

    # Ki?m tra xem y�u c?u th�nh c�ng hay kh�ng
    if response.status_code == 200:
        # Truy c?p gi� tr? t? ph?n h?i
        value = response.text
        print("Gi� tr? t? Thingspeak:", value)
        
        # Ki?m tra gi� tr? v� �i?u khi?n ch�n P9_41
        if value == '1':
            GPIO.output("P9_41", GPIO.HIGH)
        else:
            GPIO.output("P9_41", GPIO.LOW)
    else:
        print("Y�u c?u kh�ng th�nh c�ng. M? l?i:", response.status_code)
    
    # Ch? 1 gi�y tr�?c khi g?i y�u c?u ti?p theo
    time.sleep(1)
