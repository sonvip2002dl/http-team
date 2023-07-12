import pandas as pd
import numpy as np
from sklearn.linear_model import LinearRegression
import matplotlib.pyplot as plt
import csv
import time
import requests
import json
import glob
import os

url_pm25 = 'https://api.thingspeak.com/update?api_key=PC1RY78Z1CKPTBFR'
first_run = True

def download_data():
    global first_run
    if first_run:
        url = "https://api.thingspeak.com/channels/2007956/feeds.json?api_key=NDVML0QGGQINNVYO&results=100000" # dòng này chạy lần đầu để down tất cả dữ liệu có về
        first_run = False
    else:
        url = "https://api.thingspeak.com/channels/2007956/feeds.json?api_key=NDVML0QGGQINNVYO&results=1"  # dòng này để ghi thêm # phải dùng cả 2 dòng. nếu chỉ dùng dòng trên thì nó ghi số lượng lớn có thể lỗi # nếu dùng dòng dưới thì chỉ ghi dữ liệu tại thời điểm code chạy. tắt đi là dữ liệu bị ngắt quãng
        
    headers = {'Pragma': 'no-cache'}
    # Send GET request to Thingspeak API
    response = requests.get(url, headers=headers)
    data = response.json()

    # Extract required fields from the response data
    feeds = data["feeds"]
    fields = ["created_at", "entry_id", "field1", "field2", "field3", "field4", "field5", "field6"]
    extracted_data = [[feed[field] for field in fields] for feed in feeds]

    # Save data to a CSV file
    filename = f"data_{time.time()}.csv"
    with open(filename, "w", newline="") as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(fields)  # Write header
        writer.writerows(extracted_data)

    print(f"Đã tải xuống {len(extracted_data)} dữ liệu vào File ===> {filename}.")

    merge_files(fields)

def merge_files(fields):
    csv_files = glob.glob("data_*.csv")

    if len(csv_files) == 0:
        return

    merged_filename = "merged_data.csv"

    merged_data = []

    for file in csv_files:
        with open(file, "r") as csvfile:
            reader = csv.reader(csvfile)
            try:
                next(reader)  # Skip header
            except StopIteration:
                continue  # Skip empty files
            data = [row for row in reader]
            merged_data.extend(data)

    # Load existing data from the merged file if it exists
    existing_data = []
    if os.path.exists(merged_filename):
        with open(merged_filename, "r") as merged_file:
            reader = csv.reader(merged_file)
            try:
                next(reader)  # Skip header
                existing_data = [row for row in reader]
            except StopIteration:
                pass  # Empty merged file

    # Filter and append new data to existing data
    last_entry_id = int(existing_data[-1][1]) if existing_data else -1
    merged_data = [row for row in merged_data if int(row[1]) > last_entry_id]
    merged_data = existing_data + merged_data

    # Remove duplicates based on entry ID
    unique_data = []
    unique_entry_ids = set()

    for entry in merged_data:
        entry_id = int(entry[1])
        if entry_id not in unique_entry_ids:
            unique_entry_ids.add(entry_id)
            unique_data.append(entry)

    # Sort data by entry ID
    unique_data.sort(key=lambda x: int(x[1]))

    # Save merged data to file
    with open(merged_filename, "w", newline="") as merged_file:
        writer = csv.writer(merged_file)
        writer.writerow(fields)  # Write header
        writer.writerows(unique_data)

    print(f"Đã lưu dữ liệu vào File ===> {merged_filename}.")

    # Remove individual CSV files
    for file in csv_files:
        os.remove(file)
while True:
    download_data()
    time.sleep(1)
    field1_data = []
    # Đọc dữ liệu từ file merged_data.csv
    with open('merged_data.csv', 'r') as csvfile:
        reader = csv.DictReader(csvfile)
        for row in reader:
            trung_binh = float(row['field1'])
            field1_data.append(trung_binh)

    # Tính trung bình cửa sổ dữ liệu và làm tròn đến chữ số thập phân thứ hai
    lay_them = 60  # 17 và 18 ok  # >19 sai số lớn
    buoc_nhay = 180  # tương ứng với khoảng thời gian cần dự đoán # Thời gian dự đoán = 20x30 = 600 giây = 10 phút
    do_truot = buoc_nhay + lay_them  
    du = (len(field1_data) - lay_them) % buoc_nhay
    dulieu_trungbinh = []
    print("Số dữ liệu dư là: " + str(du))

    for i in range(du, len(field1_data) - buoc_nhay-1, buoc_nhay):
        cua_so = field1_data[i:i+do_truot] 
        if len(cua_so) == do_truot:
            trung_binh = round(sum(cua_so) / do_truot, 2)
            dulieu_trungbinh.append(trung_binh)

    print("Cửa sổ lấy mẫu là: " + str(len(cua_so)))
    time.sleep(1)
    # Lưu dữ liệu vào file CSV nếu cua_so có độ dài là 40
    if du == 0:
        with open('dulieu_trungbinh.csv', 'w', newline='') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow(['Trung_binh'])
            writer.writerows(zip(dulieu_trungbinh))
            print("Đang phân tích...")
        
        # Đọc dữ liệu từ file CSV
        df = pd.read_csv('dulieu_trungbinh.csv')

        # Chuẩn bị dữ liệu cho mô hình
        X = df['Trung_binh'].values[:-1].reshape(-1, 1)
        y = df['Trung_binh'].values[1:]

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

        # Tính độ lỗi trung bình bình phương trên tập kiểm tra và tập val
        mse_test = np.mean((y_pred_test - y_test) ** 2)
        mse_val = np.mean((y_pred_val - y_val) ** 2)

        # Tính RMSE bằng cách lấy căn bậc hai của MSE
        rmse_test = np.sqrt(mse_test)
        rmse_val = np.sqrt(mse_val)

        # In giá trị dự đoán cuối cùng
        final_prediction = y_pred_test[-1]
        print(f"Giá trị dự đoán: {final_prediction}")
        import pandas as pd

        # Đọc file CSV
        df = pd.read_csv('merged_data.csv')

        # Lấy 30 và 10 dòng gần nhất
        recent_data30 = df.tail(30)
        recent_data10 = df.tail(10)
        # Tính tổng của field1
        total_field1 = recent_data30['field1'].sum()
        total_field2 = recent_data10['field1'].sum()
        gia_tri_thuc = (final_prediction*40 - total_field2)/30
        print("Tổng của 30 dữ liệu gần nhất là:", total_field1)
        print("Tổng của 10 dữ liệu gần nhất là:", total_field2)
        print("Giá trị 10 phút tới: ", gia_tri_thuc)

        # Vẽ biểu đồ so sánh giá trị thực và giá trị dự đoán trên tập kiểm tra
        plt.figure()
        plt.plot(y_test, 'b')
        plt.plot(y_pred_test, 'r')
        plt.xlabel('Thời điểm')
        plt.ylabel('Nồng độ bụi')
        plt.legend(['Giá trị thực', 'Giá trị dự đoán'])
        plt.title(f'Độ lỗi trung bình trên tập kiểm tra: {rmse_test:.3f}')
        #plt.show()
        #plt.pause(10)  # Tạm dừng chương trình trong 10 giây để xem biểu đồ
        #plt.close()    # Đóng biểu đồ sau khi tạm dừng
        print(f'Độ lỗi trung bình trên tập kiểm tra: {rmse_test:.2f}')
        print(f'Độ lỗi trung bình bình phương trên tập kiểm tra: {mse_test:.2f}')
        requests.get(url_pm25 + '&field1' + '=' + str(gia_tri_thuc) + '&field2' + '='+ str(rmse_test))
        print(f"Đã gửi dữ liệu lên Thingsgpeak")
        print(f'====================================================')
        time.sleep(25)