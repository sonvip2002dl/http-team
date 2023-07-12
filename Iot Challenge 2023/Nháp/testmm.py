import requests
import pandas as pd
import numpy as np

# Lấy dữ liệu trung bình của 15 ngày trước đó từ Thingspeak
url = 'https://api.thingspeak.com/channels/2007956/fields/1.json?average=1440&days=15'
response = requests.get(url)
data = response.json()['feeds']

# Tạo DataFrame từ dữ liệu JSON
df = pd.DataFrame(data)

# Chuyển đổi định dạng ngày tháng và đặt cột thời gian làm chỉ mục
df['created_at'] = pd.to_datetime(df['created_at'])
df.set_index('created_at', inplace=True)

# Lấy dữ liệu trung bình của trường 1
data = df['field1'].values

# Chia dữ liệu thành tập huấn luyện và tập kiểm tra
train_data = data[:-1]
test_data = data[-1:]

# Chuẩn hóa dữ liệu
from sklearn.preprocessing import MinMaxScaler

scaler = MinMaxScaler(feature_range=(0, 1))
train_data = scaler.fit_transform(train_data.reshape(-1, 1))
test_data = scaler.transform(test_data.reshape(-1, 1))

# Chia dữ liệu thành các chuỗi con
def create_dataset(X, look_back=1):
    Xs, ys = [], []
    for i in range(len(X) - look_back):
        Xs.append(X[i:i + look_back])
        ys.append(X[i + look_back])
    return np.array(Xs), np.array(ys)

look_back = 15  # Số ngày quan sát trước đó
X_train, y_train = create_dataset(train_data, look_back)
X_test, y_test = create_dataset(test_data, look_back)
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import LSTM, Dense, Dropout

model = Sequential()
model.add(LSTM(64, activation='relu', input_shape=(look_back, 1)))
model.add(Dropout(0.2))
model.add(Dense(1))

model.compile(optimizer='adam', loss='mse')
model.fit(X_train, y_train, epochs=50, batch_size=16, verbose=2)
