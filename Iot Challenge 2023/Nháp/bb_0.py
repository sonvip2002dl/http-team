import serial
import struct
import requests
import time

port = 'COM4'
baudrate = 9600
channel_write_key = 'L7FFH2NQ82HULURW'

try:
    ser = serial.Serial(port, baudrate)
    
    while True:
        try:
            data = ser.read(10)
            
            if data[0] == 0xAA and data[1] == 0xC0:
                pm25 = struct.unpack('<H', data[2:4])[0] / 10.0
                pm10 = struct.unpack('<H', data[4:6])[0] / 10.0
                
                url = f'https://api.thingspeak.com/update.json'
                params = {
                    'api_key': channel_write_key,
                    'field1': pm25,
                    'field2': pm10
                }
                response = requests.post(url, params=params)
                if response.ok:
                    pass
                else:
                    pass

        except serial.SerialException as e:
            print(f"{str(e)}")
            break
        
        except struct.error as e:
            print(f"{str(e)}")
            break
        
        time.sleep(20)

except serial.SerialException as e:
    print(f" {str(e)}")

finally:
    ser.close()
