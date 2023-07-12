

# Phương pháp dự đoán trong mã bạn đưa ra là hồi quy tuyến tính (Linear Regression). 
# Đoạn mã sử dụng thư viện scikit-learn để tạo mô hình hồi quy tuyến tính và dự đoán nồng độ PM2.5 và PM10 
# trong 5 ngày tiếp theo dựa trên dữ liệu đã được lấy từ API ThingSpeak.

import requests
import json
import openpyxl
import pandas as pd
from sklearn.linear_model import LinearRegression
import time

pm25_day = []
pm10_day = []
# Khai báo thông tin API key và địa chỉ URL ThingSpeak
url_pm25 = 'https://api.thingspeak.com/update?api_key=PC1RY78Z1CKPTBFR'
url_pm10 = 'https://api.thingspeak.com/update?api_key=7BFMBUXZPY7FH4P1'

while True:
    # Lấy dữ liệu từ API
    url = 'https://api.thingspeak.com/channels/2007956/feeds.json?results=10000'
    response = requests.get(url)
    data = json.loads(response.text)

    # Tạo DataFrame từ dữ liệu
    df = pd.DataFrame(data['feeds'])
    df = df[['field1', 'field2']]
    df.columns = ['pm25', 'pm10']
    df['pm25'] = pd.to_numeric(df['pm25'], errors='coerce')
    df['pm10'] = pd.to_numeric(df['pm10'], errors='coerce')

    # Mở rộng tập dữ liệu lên 5 ngày
    df = pd.concat([df]*6, ignore_index=True)
    for i in range(1, 6):
        df['pm25'].iloc[-i*24:] = None
        df['pm10'].iloc[-i*24:] = None
    df.dropna(inplace=True)

    # Xử lý dữ liệu
    X = df[['pm25', 'pm10']]
    y_pm25 = df['pm25']
    y_pm10 = df['pm10']

    # Tạo mô hình hồi quy tuyến tính
    model_pm25 = LinearRegression().fit(X, y_pm25)
    model_pm10 = LinearRegression().fit(X, y_pm10)

    # Dự đoán nồng độ PM2.5 và PM10 trong 5 ngày tiếp theo
    for i in range(1, 6):
        pm25_prediction = round(model_pm25.predict([[X.iloc[-i*24]['pm25'], X.iloc[-i*24]['pm10']]])[0], 1)
        pm10_prediction = round(model_pm10.predict([[X.iloc[-i*24]['pm25'], X.iloc[-i*24]['pm10']]])[0], 1)
        print("Dự đoán nồng độ PM2.5 trong ngày thứ", i, "tiếp theo:", pm25_prediction)
        print("Dự đoán nồng độ PM10 trong ngày thứ", i, "tiếp theo:", pm10_prediction)
        pm25_day.append(pm25_prediction)
        pm10_day.append(pm10_prediction)
        
    # Gửi giá trị dự đoán PM2.5 và PM10 đến ThingSpeak
    requests.get(url_pm25 + '&field1' + '=' + str(pm25_day[0]) + '&field2' + '=' + str(pm25_day[1]) + '&field3' + '=' + str(pm25_day[2]) + '&field4' + '=' + str(pm25_day[3]) + '&field5' + '=' + str(pm25_day[4]))
    requests.get(url_pm10 + '&field1' + '=' + str(pm10_day[0]) + '&field2' + '=' + str(pm10_day[1]) + '&field3' + '=' + str(pm10_day[2]) + '&field4' + '=' + str(pm10_day[3]) + '&field5' + '=' + str(pm10_day[4]))
   
    # Lưu giá trị vào file Excel
    df_excel = pd.DataFrame({'pm25': pm25_day, 'pm10': pm10_day})
    import pandas as pd
    df_excel.to_excel('du-doan-5-ngay-tiep-theo.xlsx') 

    # Clear giá trị của pm25_day và pm10_day
    pm25_day.clear()
    pm10_day.clear()
    # Giải phóng bộ nhớ
    del df, X, y_pm25, y_pm10, model_pm25, model_pm10

    # Đợi 20 giây trước khi chạy lại đoạn code   # 1 giờ = 60x60 = 3600 giây
    time.sleep(15)
