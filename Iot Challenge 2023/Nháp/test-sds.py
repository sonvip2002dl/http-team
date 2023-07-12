import serial
import struct
import requests
import time

# Cấu hình cổng COM
port = 'COM4'
baudrate = 9600
channel_write_key = 'L7FFH2NQ82HULURW'

try:
    # Mở kết nối serial
    ser = serial.Serial(port, baudrate)
    
    while True:
        try:
            # Đọc dữ liệu từ cổng COM
            data = ser.read(10)
            
            # Kiểm tra header và checksum
            if data[0] == 0xAA and data[1] == 0xC0:
                # Giải mã dữ liệu
                pm25 = struct.unpack('<H', data[2:4])[0] / 10.0
                pm10 = struct.unpack('<H', data[4:6])[0] / 10.0
                
                # In giá trị lên màn hình
                print(f"PM2.5: {pm25} µg/m³")
                print(f"PM10: {pm10} µg/m³")
                
                # Gửi dữ liệu lên Thingspeak
                url = f'https://api.thingspeak.com/update.json'
                params = {
                    'api_key': channel_write_key,
                    'field1': pm25,
                    'field2': pm10
                }
                response = requests.post(url, params=params)
                if response.ok:
                    print("Dữ liệu đã được gửi lên Thingspeak.")
                else:
                    print("Lỗi khi gửi dữ liệu lên Thingspeak.")

        except serial.SerialException as e:
            # Xử lý lỗi ngoại lệ SerialException
            print(f"Lỗi Serial: {str(e)}")
            break
        
        except struct.error as e:
            # Xử lý lỗi ngoại lệ struct.error
            print(f"Lỗi giải mã dữ liệu: {str(e)}")
            break
        
        # Tạm dừng 20 giây
        time.sleep(20)

except serial.SerialException as e:
    # Xử lý lỗi ngoại lệ SerialException khi mở kết nối serial
    print(f"Lỗi Serial: {str(e)}")

finally:
    # Đóng kết nối serial
    ser.close()
