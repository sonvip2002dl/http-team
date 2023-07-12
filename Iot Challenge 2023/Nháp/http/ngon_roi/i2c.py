import smbus
import urllib
import time

BME280_ADDRESS = 0x76

I2C_DEVICE = '/dev/i2c-1'

API_KEY = 'I9JOBOQVI02E386R'
THINGSPEAK_URL = 'https://api.thingspeak.com/update'

bus = smbus.SMBus(1)

def read_bme280_data():
    bus.write_byte_data(BME280_ADDRESS, 0xf7, 0x00)
    
    data = bus.read_i2c_block_data(BME280_ADDRESS, 0xfa, 3)
    temp = ((data[0] << 16) | (data[1] << 8) | data[2]) / 100.0
    
    data = bus.read_i2c_block_data(BME280_ADDRESS, 0xfd, 2)
    hum = ((data[0] << 8) | data[1]) / 1024.0
    
    data = bus.read_i2c_block_data(BME280_ADDRESS, 0xf7, 3)
    pres = ((data[0] << 16) | (data[1] << 8) | data[2]) / 256.0
    
    return temp, hum, pres

def send_data_to_thingspeak(temp, hum, pres):
    url = '{}?api_key={}&field1={}&field2={}&field3={}'.format(THINGSPEAK_URL, API_KEY, temp, hum, pres)
    response = urllib.urlopen(url)
    
while True:
    temp, hum, pres = read_bme280_data()
    
    send_data_to_thingspeak(temp, hum, pres)
    
    time.sleep(15)
