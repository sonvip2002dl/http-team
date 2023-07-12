import numpy as np
from sklearn.linear_model import LinearRegression
import requests
from datetime import datetime, timedelta

# Tính thời gian hiện tại lùi về trước 1 ngày
current_time = datetime.now()
start_time = current_time - timedelta(days=1)

# Chuẩn bị thời gian trong định dạng chuẩn cho URL API
start_time_str = start_time.strftime('%Y-%m-%dT%H:%M:%SZ')

# Tạo URL API mới với thời gian cập nhật
api_url = f'https://api.thingspeak.com/channels/2007956/fields/1.json?start=2023-04-25T00:00:00Z&end={start_time_str}&timezone=Asia/Hanoi&round=2&average=1440&api_key=L7FFH2NQ82HULURW'

# Load dữ liệu từ kênh ThingSpeak
response = requests.get(api_url)
data = response.json()['feeds']

# Chuẩn bị dữ liệu cho mô hình
X = np.array([[1, float(data[i]['field1'])] for i in range(len(data)-1)])
y = np.array([float(data[i]['field1']) for i in range(1, len(data))])

# Huấn luyện mô hình
model = LinearRegression()
model.fit(X, y)

# Dự đoán giá trị của 5 lần tiếp theo
num_predictions = 5
X_pred = np.array([[1, y[-1]]])  # Sử dụng giá trị hiện tại làm feature

predictions = []
for _ in range(num_predictions):
    y_pred = model.predict(X_pred)
    predictions.append(y_pred)
    X_pred = np.array([[1, y_pred]])  # Sử dụng giá trị dự đoán làm feature cho lần tiếp theo

# Hiển thị giá trị dự đoán
for i, pred in enumerate(predictions, 1):
    print(f'Giá trị dự đoán {i}: {pred}')
