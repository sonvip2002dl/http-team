import requests
import pandas as pd
import numpy as np
from sklearn.preprocessing import MinMaxScaler
from keras.models import Sequential
from keras.layers import LSTM, Dense
import time
url_pm25 = 'https://api.thingspeak.com/update?api_key=7BFMBUXZPY7FH4P1'
while True:
    # Đọc dữ liệu từ tệp dulieu_trungbinh.csv
    df = pd.read_csv("dulieu_trungbinh.csv")

    # Chuyển đổi kiểu dữ liệu của cột dữ liệu (field1) sang kiểu số thực
    df['Trung_binh'] = df['Trung_binh'].astype(float)

    # Chuẩn bị dữ liệu đầu vào cho mô hình học máy
    scaler = MinMaxScaler(feature_range=(0, 1))
    dataset = scaler.fit_transform(df['Trung_binh'].values.reshape(-1, 1))
    look_back = 20  # Số lượng các quan sát trước đó sẽ được sử dụng để dự đoán quan sát hiện tại
    X, y = [], []
    for i in range(len(dataset) - look_back):
        X.append(dataset[i:(i + look_back), 0])
        y.append(dataset[i + look_back, 0])
    X, y = np.array(X), np.array(y)
    X = np.reshape(X, (X.shape[0], X.shape[1], 1))

    # Huấn luyện mô hình học máy (sử dụng LSTM)
    model = Sequential()
    model.add(LSTM(units=50, return_sequences=True, input_shape=(X.shape[1], 1)))
    model.add(LSTM(units=50))
    model.add(Dense(units=1))
    model.compile(optimizer='adam', loss='mean_squared_error')
    model.fit(X, y, epochs=50, batch_size=32)

    # Dự đoán nồng độ bụi trong 5 phút tiếp theo
    last_obs = scaler.transform(np.array(df['Trung_binh'][-look_back:]).reshape(-1, 1))
    next_5_min = np.array(last_obs).reshape(1, look_back, 1)
    predicted = model.predict(next_5_min)
    predicted_value = scaler.inverse_transform(predicted)[0][0]
    print("Dự đoán nồng độ bụi trong 5 phút tiếp theo:", predicted_value)
    requests.get(url_pm25 + '&field1' + '=' + str( predicted_value))
    print(f"Đã gửi dữ liệu lên Thingsgpeak")
    print(f'====================================================')
    # Đợi 5 phút trước khi chạy lại mô hình
    time.sleep(300)
