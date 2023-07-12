# -*- coding: utf-8 -*-
import requests
import time
import Adafruit_BBIO.GPIO as GPIO

url = "https://api.thingspeak.com/channels/1956999/field/1/last"

# C?u h?nh chân P9_41 là ð?u ra
GPIO.setup("P9_41", GPIO.OUT)

while True:
    # G?i yêu c?u GET ð?n URL và lýu tr? ph?n h?i trong bi?n response
    response = requests.get(url)

    # Ki?m tra xem yêu c?u thành công hay không
    if response.status_code == 200:
        # Truy c?p giá tr? t? ph?n h?i
        value = response.text
        print("Giá tr? t? Thingspeak:", value)
        
        # Ki?m tra giá tr? và ði?u khi?n chân P9_41
        if value == '1':
            GPIO.output("P9_41", GPIO.HIGH)
        else:
            GPIO.output("P9_41", GPIO.LOW)
    else:
        print("Yêu c?u không thành công. M? l?i:", response.status_code)
    
    # Ch? 1 giây trý?c khi g?i yêu c?u ti?p theo
    time.sleep(1)
