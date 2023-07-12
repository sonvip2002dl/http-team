import requests
import pandas as pd
import numpy as np
import time
from keras.models import Sequential
from keras.layers import LSTM, Dense
from sklearn.preprocessing import MinMaxScaler
while True:
    # Lấy dữ liệu từ link
    response = requests.get("https://api.thingspeak.com/channels/2007956/feeds.json?results=10000")
    data = response.json()

    # Chuyển đổi dữ liệu thành DataFrame
    df = pd.DataFrame(data['feeds'])
    df = df.drop(columns=['created_at', 'entry_id'])

    # Chuyển đổi kiểu dữ liệu của cột dữ liệu (field1) sang kiểu số thực
    df['field1'] = df['field1'].astype(float)

    # Chuẩn bị dữ liệu đầu vào cho mô hình học máy
    X = df['field1'].values
    y = df['field1'].shift(-1).values.reshape(-1, 1)[:-1]

    # Chuẩn hóa dữ liệu
    scaler = MinMaxScaler(feature_range=(0, 1))
    X = scaler.fit_transform(X.reshape(-1, 1))
    y = scaler.transform(y)

    # Chuyển đổi dữ liệu thành dạng chuỗi thời gian (time series)
    X_time_series = []
    y_time_series = []
    time_steps = 5
    for i in range(time_steps, len(X)):
        X_time_series.append(X[i-time_steps:i, 0])
        y_time_series.append(y[i-1, 0])

    X_time_series = np.array(X_time_series)
    y_time_series = np.array(y_time_series)

    # Chia dữ liệu thành tập huấn luyện và tập kiểm tra
    train_size = int(len(X_time_series) * 0.7)
    X_train = X_time_series[:train_size]
    y_train = y_time_series[:train_size]
    X_test = X_time_series[train_size:]
    y_test = y_time_series[train_size:]

    # Xây dựng mô hình LSTM
    model = Sequential()
    model.add(LSTM(units=64, input_shape=(X_train.shape[1], 1)))
    model.add(Dense(units=1))
    model.compile(optimizer='adam', loss='mean_squared_error')

    # Huấn luyện mô hình LSTM
    model.fit(X_train.reshape(X_train.shape[0], X_train.shape[1], 1), y_train, epochs=100, batch_size=32)

    # Dự đoán nồng độ bụi trong 5 phút tiếp theo
    next_5_min = X_test[-1].reshape(1, X_test.shape[1], 1)
    predicted = model.predict(next_5_min)
    predicted = scaler.inverse_transform(predicted)
    print("Dự đoán nồng độ bụi trong 5 phút tiếp theo:", predicted[0][0])
    time.sleep(5)