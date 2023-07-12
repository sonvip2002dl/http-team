import requests
import pandas as pd
import time
# Khai báo thông tin API key và địa chỉ URL ThingSpeak
url1 = 'https://api.thingspeak.com/update?api_key=EHL7YVL5TSME8BWE'

while True:
# Hàm tính giá trị trung bình của 20 giá trị gần nhất
    def calculate_average(data, n=20):
        if len(data) < n:
            n = len(data)
        return data[-n:].mean()
    # Gửi yêu cầu HTTP để lấy dữ liệu từ API
    url = 'https://api.thingspeak.com/channels/2007956/feeds.json?results=200000'
    response = requests.get(url)
    data = response.json()

    # Chuyển đổi dữ liệu thành dataframe
    df = pd.DataFrame(data['feeds'])

    # Chọn cột 'field1' (PM2.5) và 'field2' (PM10)
    df = df[['field1', 'field2']]

    # Đổi tên cột thành 'pm25' và 'pm10'
    df = df.rename(columns={'field1': 'pm25', 'field2': 'pm10'})

    # Xóa các dòng có giá trị thiếu
    df.dropna(inplace=True)

    # Chuyển đổi kiểu dữ liệu thành số thực
    df = df.astype(float)

    # Lấy dữ liệu hiện tại
    current_data = df.iloc[-1]

    # Lấy dữ liệu trước đó
    previous_data = df.iloc[:-1]

    # Tính giá trị trung bình của 20 giá trị gần nhất
    average_values = calculate_average(previous_data)

    # Tính độ chênh lệch giữa giá trị hiện tại và giá trị trung bình
    diff_pm25 = current_data['pm25'] - average_values['pm25']
    diff_pm10 = current_data['pm10'] - average_values['pm10']

    # Xác định xu hướng
    trend_pm25 = "" # 0 - 1 -2 giảm - ổn định - tăng
    if diff_pm25 > 0.03:
        trend_pm25 = "2"
    elif diff_pm25 < -0.03:
        trend_pm25 = "0"
    else:
        trend_pm25 = "1"

    trend_pm10 = ""  # 0 - 1 -2 giảm - ổn định - tăng
    if diff_pm10 > 0.03:
        trend_pm10 = "2"
    elif diff_pm10 < -0.03:
        trend_pm10 = "0"
    else:
        trend_pm10 = "1"

    # In kết quả dự đoán
    print("Nồng độ PM2.5 trong khoảng thời gian tiếp theo có xu hướng:", trend_pm25)
    print("Nồng độ PM10 trong khoảng thời gian tiếp theo có xu hướng:", trend_pm10)
     # Gửi giá trị dự đoán PM2.5 và PM10 đến ThingSpeak
    requests.get(url1 + '&field1' + '=' + str(trend_pm25) + '&field2' + '=' + str(trend_pm10) )
    time.sleep(15)
