import smbus
import requests
import time

# Địa chỉ I2C của cảm biến BME280
BME280_ADDRESS = 0x76

# Đường dẫn file thiết bị I2C
I2C_DEVICE = '/dev/i2c-1'

# ThingSpeak API Key và URL
API_KEY = 'I9JOBOQVI02E386R'
THINGSPEAK_URL = f'https://api.thingspeak.com/update?api_key={API_KEY}'

# Khởi tạo bus I2C
bus = smbus.SMBus(1)

def read_bme280_data():
    # Gửi lệnh đọc dữ liệu từ cảm biến BME280
    bus.write_byte_data(BME280_ADDRESS, 0xf7, 0x00)
    
    # Đọc dữ liệu nhiệt độ
    data = bus.read_i2c_block_data(BME280_ADDRESS, 0xfa, 3)
    temp = ((data[0] << 16) | (data[1] << 8) | data[2]) / 100
    
    # Đọc dữ liệu độ ẩm
    data = bus.read_i2c_block_data(BME280_ADDRESS, 0xfd, 2)
    hum = ((data[0] << 8) | data[1]) / 1024
    
    # Đọc dữ liệu áp suất
    data = bus.read_i2c_block_data(BME280_ADDRESS, 0xf7, 3)
    pres = ((data[0] << 16) | (data[1] << 8) | data[2]) / 256
    
    return temp, hum, pres

def send_data_to_thingspeak(temp, hum, pres):
    # Tạo các tham số dữ liệu gửi lên ThingSpeak
    params = {
        'api_key': API_KEY,
        'field1': temp,
        'field2': hum,
        'field3': pres
    }
    
    # Gửi yêu cầu POST đến ThingSpeak
    response = requests.post(THINGSPEAK_URL, params=params)
    
    # Kiểm tra kết quả
    if response.status_code == 200:
        print('Dữ liệu đã được gửi lên ThingSpeak thành công.')
    else:
        print('Gửi dữ liệu lên ThingSpeak không thành công.')

while True:
    # Đọc giá trị từ cảm biến BME280
    temp, hum, pres = read_bme280_data()
    
    # Gửi dữ liệu lên ThingSpeak
    send_data_to_thingspeak(temp, hum, pres)
    
    # Chờ 15 giây trước khi đọc lại dữ liệu và gửi lên ThingSpeak
    time.sleep(15)
