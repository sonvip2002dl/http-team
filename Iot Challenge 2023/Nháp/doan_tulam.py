import pandas as pd
import numpy as np
from sklearn.linear_model import LinearRegression
import matplotlib.pyplot as plt

# Đọc dữ liệu từ file CSV
df = pd.read_csv('dulieu_trungbinh.csv')

# Chuẩn bị dữ liệu cho mô hình
X = df['Trung binh'].values[:-1].reshape(-1, 1)
y = df['Trung binh'].values[1:]

# Chia dữ liệu thành tập huấn luyện và tập kiểm tra
trainRatio = 0.7
valRatio = 0.15
testRatio = 0.15
train_size = int(trainRatio * len(X))
val_size = int(valRatio * len(X))

X_train = X[:train_size]
y_train = y[:train_size]
X_val = X[train_size:train_size+val_size]
y_val = y[train_size:train_size+val_size]
X_test = X[train_size+val_size:]
y_test = y[train_size+val_size:]

# Huấn luyện mô hình hồi quy tuyến tính
model = LinearRegression()
model.fit(X_train, y_train)

# Dự đoán trên tập kiểm tra và tập val
y_pred_test = model.predict(X_test)
y_pred_val = model.predict(X_val)

# Tính độ lỗi trung bình trên tập kiểm tra và tập val
mse_test = np.mean((y_pred_test - y_test) ** 2)
mse_val = np.mean((y_pred_val - y_val) ** 2)

# Vẽ biểu đồ so sánh giá trị thực và giá trị dự đoán trên tập kiểm tra
plt.figure()
plt.plot(y_test, 'b')
plt.plot(y_pred_test, 'r')
plt.xlabel('Thời điểm')
plt.ylabel('Nồng độ bụi')
plt.legend(['Giá trị thực', 'Giá trị dự đoán'])
plt.title(f'Độ lỗi trung bình trên tập kiểm tra: {mse_test:.2f}')
plt.show()
