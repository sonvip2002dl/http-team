import numpy as np
import pandas as pd
import requests
import time
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import LSTM, Dense
from sklearn.preprocessing import MinMaxScaler

pm25_day = []
pm10_day = []
# Khai báo thông tin API key và địa chỉ URL ThingSpeak
api_key = "PC1RY78Z1CKPTBFR"
url_pm25 = f"https://api.thingspeak.com/update?api_key={api_key}"

# Lấy dữ liệu từ API
url = "https://api.thingspeak.com/channels/2007956/fields/1.json?start=2023-04-30T00:00:00Z&end=2030-05-19T12:00:00Z&timezone=Asia/Hanoi&round=2&average=60&api_key=L7FFH2NQ82HULURW"
response = requests.get(url)
data = response.json()["feeds"]

# Chuẩn bị dữ liệu
df = pd.DataFrame(data)
df["created_at"] = pd.to_datetime(df["created_at"])
df = df.set_index("created_at")
pm25 = df["field1"].values

# Chuẩn hóa dữ liệu
scaler = MinMaxScaler(feature_range=(0, 1))
pm25 = scaler.fit_transform(pm25.reshape(-1, 1))

# Hàm tạo dữ liệu cho LSTM
def create_dataset(dataset, look_back=1):
    X, Y = [], []
    for i in range(len(dataset) - look_back):
        X.append(dataset[i:i+look_back])
        Y.append(dataset[i+look_back])
    return np.array(X), np.array(Y)

# Chuẩn bị dữ liệu cho LSTM
look_back = 24  # Sử dụng 24 giờ gần nhất để dự đoán
X, Y = create_dataset(pm25, look_back)

# Reshape lại định dạng dữ liệu cho LSTM
X = np.reshape(X, (X.shape[0], X.shape[1], 1))

# Xây dựng mô hình LSTM
model = Sequential()
model.add(LSTM(units=50, return_sequences=True, input_shape=(look_back, 1)))
model.add(LSTM(units=50))
model.add(Dense(units=1))
model.compile(optimizer="adam", loss="mean_squared_error")

# Huấn luyện mô hình
model.fit(X, Y, epochs=100, batch_size=32)

# Dự đoán nồng độ bụi PM2.5 trong ngày tiếp theo
last_day = pm25[-look_back:].reshape(1, look_back, 1)
predicted = model.predict(last_day)
pm25_day.append(predicted)
# Chuẩn hóa lại giá trị dự đoán
predicted = scaler.inverse_transform(predicted)

# Gửi giá trị dự đoán PM2.5 đến ThingSpeak
requests.get(url_pm25 + '&field1' + '=' + str(predicted[0][0]))

print("Dự đoán nồng độ bụi PM2.5 trong ngày tiếp theo:")
print(predicted)
print(pm25_day)
time.sleep(3600)
