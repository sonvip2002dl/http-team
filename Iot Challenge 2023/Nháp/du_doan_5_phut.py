import requests
import pandas as pd
import numpy as np
import time
from sklearn.linear_model import LinearRegression

# Lấy dữ liệu từ link
response = requests.get("https://api.thingspeak.com/channels/2007956/feeds.json?results=100000")
data = response.json()

# Chuyển đổi dữ liệu thành DataFrame
df = pd.DataFrame(data['feeds'])
df = df.drop(columns=['created_at', 'entry_id'])

# Chuyển đổi kiểu dữ liệu của cột dữ liệu (field1) sang kiểu số thực
df['field1'] = df['field1'].astype(float)

# Chuẩn bị dữ liệu đầu vào cho mô hình học máy
X = df['field1'].to_numpy().reshape(-1, 1)
y = df['field1'].shift(-5).to_numpy().reshape(-1, 1)

# Xóa các hàng cuối cùng vì chúng không có giá trị nồng độ bụi dự đoán
X = X[:-5]
y = y[:-5]

# Huấn luyện mô hình học máy (sử dụng Linear Regression)
model = LinearRegression().fit(X, y)

# Dự đoán nồng độ bụi trong 5 phút tiếp theo
next_5_min = np.array([df['field1'].iloc[-1]]).reshape(1, -1)
predicted = model.predict(next_5_min)
print("Dự đoán nồng độ bụi trong 5 phút tiếp theo:", predicted[0][0])
time.sleep(300)
