import requests
import pandas as pd
from sklearn.linear_model import LinearRegression
from sklearn.model_selection import train_test_split

# Lấy dữ liệu từ Thingspeak
url = 'https://api.thingspeak.com/channels/2007956/feeds.json?results=8000'
response = requests.get(url)
data = response.json()['feeds']
df = pd.DataFrame(data)

# Chọn các feature cần thiết và chuyển đổi kiểu dữ liệu
X = df[['field2', 'field4', 'field5']].astype(float)
y = df['field1'].astype(float)

# Chia thành tập huấn luyện và tập kiểm tra
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.3, random_state=42)

# Áp dụng mô hình hồi quy tuyến tính
lr = LinearRegression()
lr.fit(X_train, y_train)

# Dự đoán nồng độ bụi PM2.5 trong 1 giờ tiếp theo
pm10 = 100  # giá trị nồng độ bụi PM10
humidity = 70  # giá trị độ ẩm
temperature = 25  # giá trị nhiệt độ
X_new = [[pm10, humidity, temperature]]
y_pred = lr.predict(X_new)

print(f'Nồng độ bụi PM2.5 trong 1 giờ tiếp theo là: {y_pred[0]}')
