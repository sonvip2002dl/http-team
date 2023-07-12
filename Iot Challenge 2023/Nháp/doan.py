import requests
import json
import pandas as pd
from sklearn.linear_model import LinearRegression
import time

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

    # Giải phóng bộ nhớ
    del df, X, y_pm25, y_pm10, model_pm25, model_pm10

    # Đợi 60 giây trước khi chạy lại đoạn code
    time.sleep(6)
