import numpy as np
import matplotlib.pyplot as plt
from sklearn.linear_model import LinearRegression
import requests

# Load dữ liệu từ kênh ThingSpeak
readChannelID = 2007956
readAPIKey = 'NDVML0QGGQINNVYO'
response = requests.get(f'https://api.thingspeak.com/channels/{readChannelID}/fields/1.json?api_key={readAPIKey}&results=2000')
data = response.json()['feeds']

# Chuẩn bị dữ liệu cho mô hình
X = np.array([[1, float(data[i]['field1'])] for i in range(len(data)-1)])
y = np.array([float(data[i]['field1']) for i in range(1, len(data))])

# Chia dữ liệu thành tập huấn luyện và tập kiểm tra
trainRatio = 0.7
valRatio = 0.15
testRatio = 0.15

train_size = int(trainRatio * len(y))
val_size = int(valRatio * len(y))
test_size = int(testRatio * len(y))

X_train = X[:train_size]
y_train = y[:train_size]
X_val = X[train_size:train_size+val_size]
y_val = y[train_size:train_size+val_size]
X_test = X[train_size+val_size:train_size+val_size+test_size]
y_test = y[train_size+val_size:train_size+val_size+test_size]

# Huấn luyện mô hình
model = LinearRegression()
model.fit(X_train, y_train)

# Dự đoán trên tập kiểm tra
y_pred = model.predict(X_test)

# Tính độ lỗi trung bình trên tập kiểm tra
mse_test = np.mean((y_pred - y_test)**2)

# Vẽ biểu đồ so sánh giữa giá trị thực và giá trị dự đoán
plt.plot(y_test, 'b')
plt.gca().set_prop_cycle(None)
plt.plot(y_pred, 'r')
plt.xlabel('Thời điểm')
plt.ylabel('Nồng độ bụi')
plt.legend(['Giá trị thực', 'Giá trị dự đoán'])
plt.title('Độ lỗi trung bình trên tập kiểm tra: ' + str(mse_test))
plt.show()
