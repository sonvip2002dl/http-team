import csv

# Tên tập tin CSV đầu vào
input_file = 'merged_data.csv'

# Mã entry_id tối thiểu và tối đa để lọc dữ liệu
min_entry_id = 126231
max_entry_id = 126347

# Khởi tạo giá trị tối đa và tối thiểu ban đầu
min_pm25 = float('inf')
max_pm25 = float('-inf')
min_pm10 = float('inf')
max_pm10 = float('-inf')
min_pm25_time = ""
max_pm25_time = ""
min_pm10_time = ""
max_pm10_time = ""
sum_pm25 = 0
sum_pm10 = 0
count_pm25 = 0
count_pm10 = 0

# Mở tập tin đầu vào
with open(input_file, 'r') as file_in:
    reader = csv.DictReader(file_in)

    # Lặp qua từng dòng trong tập tin CSV đầu vào
    for row in reader:
        # Kiểm tra điều kiện lọc
        entry_id = int(row['entry_id'])
        if min_entry_id <= entry_id <= max_entry_id:
            # Lấy giá trị của pm2.5 và pm10
            pm25 = float(row['field1'])
            pm10 = float(row['field2'])
            timestamp = row['created_at']

            # Cập nhật giá trị tối đa và tối thiểu
            if pm25 < min_pm25:
                min_pm25 = pm25
                min_pm25_time = timestamp
            if pm25 > max_pm25:
                max_pm25 = pm25
                max_pm25_time = timestamp
            if pm10 < min_pm10:
                min_pm10 = pm10
                min_pm10_time = timestamp
            if pm10 > max_pm10:
                max_pm10 = pm10
                max_pm10_time = timestamp

            # Cộng dồn giá trị và đếm số lượng
            sum_pm25 += pm25
            sum_pm10 += pm10
            count_pm25 += 1
            count_pm10 += 1

# Tính trung bình cộng
avg_pm25 = sum_pm25 / count_pm25
avg_pm10 = sum_pm10 / count_pm10

# In kết quả
print("Giá trị PM2.5 nhỏ nhất:", min_pm25, "thời gian:", min_pm25_time)
print("Giá trị PM2.5 lớn nhất:", max_pm25, "thời gian:", max_pm25_time)
print("Giá trị PM10 nhỏ nhất:", min_pm10, "thời gian:", min_pm10_time)
print("Giá trị PM10 lớn nhất:", max_pm10, "thời gian:", max_pm10_time)
print("Trung bình cộng PM2.5:", avg_pm25)
print("Trung bình cộng PM10:", avg_pm10)
