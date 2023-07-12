# MÔ TẢ:
# Đoạn code này để tính trung bình dữ liệu trước khi đưa vào các mô hình dự đoán

import csv
import time
while True:
    field1_data = []

    # Đọc dữ liệu từ file merged_data.csv
    with open('merged_data.csv', 'r') as csvfile:
        reader = csv.DictReader(csvfile)
        for row in reader:
            trung_binh = float(row['field1'])
            field1_data.append(trung_binh)

    # Tính trung bình cửa sổ dữ liệu và làm tròn đến chữ số thập phân thứ hai
    lay_them = 10  # 17 và 18 ok  # >19 sai số lớn
    buoc_nhay = 30  # tương ứng với khoảng thời gian cần dự đoán # Thời gian dự đoán = 20x30 = 600 giây = 10 phút
    do_truot = buoc_nhay + lay_them  
    du = (len(field1_data) - lay_them) % buoc_nhay
    dulieu_trungbinh = []
    print("Số dữ liệu dư là: " + str(du))
    for i in range(du, len(field1_data) - buoc_nhay-1, buoc_nhay):
        cua_so = field1_data[i:i+do_truot] 
        trung_binh = round(sum(cua_so) / do_truot, 2)
        dulieu_trungbinh.append(trung_binh)
    print("Cửa sổ lấy mẫu là: " + str(cua_so))  
    # Lưu dữ liệu vào file CSV
    with open('dulieu_trungbinh.csv', 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(['Trung_binh'])
        writer.writerows(zip(dulieu_trungbinh))
    print("Dữ liệu đã được lưu vào file dulieu_trungbinh.csv")
    time.sleep(1)