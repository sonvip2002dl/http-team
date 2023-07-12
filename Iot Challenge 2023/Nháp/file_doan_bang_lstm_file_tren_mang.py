import pandas as pd
import numpy as np
from sklearn.preprocessing import MinMaxScaler
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import LSTM, Dense

# Đọc dữ liệu từ file CSV
df = pd.read_csv('data1.csv')

# Chỉ lấy cột 'median'
data = df['median'].values

# Chuẩn hóa dữ liệu về khoảng [0, 1]
scaler = MinMaxScaler(feature_range=(0, 1))
data = scaler.fit_transform(data.reshape(-1, 1))

# Chuẩn bị dữ liệu huấn luyện
sequence_length = 5  # Độ dài chuỗi thời gian đầu vào
x_train = []
y_train = []
for i in range(sequence_length, len(data)):
    x_train.append(data[i-sequence_length:i, 0])
    y_train.append(data[i, 0])
x_train, y_train = np.array(x_train), np.array(y_train)

# Reshape dữ liệu huấn luyện sang định dạng [samples, time steps, features]
x_train = np.reshape(x_train, (x_train.shape[0], x_train.shape[1], 1))

# Xây dựng mô hình LSTM
model = Sequential()
model.add(LSTM(50, return_sequences=True, input_shape=(x_train.shape[1], 1)))
model.add(LSTM(50))
model.add(Dense(1))

# Compile và huấn luyện mô hình
model.compile(optimizer='adam', loss='mean_squared_error')
model.fit(x_train, y_train, epochs=50, batch_size=32)

# Dự đoán nồng độ bụi median tiếp theo
last_sequence = data[-sequence_length:].reshape(1, -1)
input_sequence = np.append(last_sequence[:, 1:], [[0]], axis=1)  # Thêm giá trị dự đoán vào cuối chuỗi
input_sequence = np.reshape(input_sequence, (1, input_sequence.shape[1], 1))
predicted_value = model.predict(input_sequence)
predicted_value = scaler.inverse_transform(predicted_value)

print('Dự đoán nồng độ bụi median tiếp theo:', predicted_value[0, 0])
