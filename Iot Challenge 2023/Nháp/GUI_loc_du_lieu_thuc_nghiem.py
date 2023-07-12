import csv
import tkinter as tk
from tkinter import messagebox

# Tạo cửa sổ giao diện Tkinter
window = tk.Tk()
window.title("Chương trình thực nghiệm BYSON")
window.geometry("400x165")

# Hàm xử lý sự kiện khi người dùng nhấn nút "Lọc"
def filter_data():
    try:
        # Đọc giá trị min_entry_id và max_entry_id từ các ô nhập liệu
        min_entry_id = int(min_entry_id_entry.get())
        max_entry_id = int(max_entry_id_entry.get())

        # Tên tập tin CSV đầu vào
        input_file = 'merged_data.csv'

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

        # Tính trung bình cộng và làm tròn đến 2 chữ số thập phân
        avg_pm25 = round(sum_pm25 / count_pm25, 2)
        avg_pm10 = round(sum_pm10 / count_pm10, 2)

        # Hiển thị kết quả trong hộp thoại thông báo
        result_message = f"Giá trị PM2.5 nhỏ nhất: {min_pm25} (Thời điểm: {min_pm25_time})\n" \
            f"Giá trị PM2.5 lớn nhất: {max_pm25} (Thời điểm: {max_pm25_time})\n" \
            f"Giá trị PM10 nhỏ nhất: {min_pm10} (Thời điểm: {min_pm10_time})\n" \
            f"Giá trị PM10 lớn nhất: {max_pm10} (Thời điểm: {max_pm10_time})\n" \
            f"Trung bình cộng PM2.5: {avg_pm25:.2f}\n" \
            f"Trung bình cộng PM10: {avg_pm10:.2f}"

        messagebox.showinfo("Kết quả", result_message)

    except ValueError:
        messagebox.showerror("Lỗi", "Vui lòng nhập giá trị khoảng là số nguyên và tồn tại")

# Tạo nhãn và ô nhập liệu cho min_entry_id
min_entry_id_label = tk.Label(window, text="Bắt đầu từ:")
min_entry_id_label.pack()
min_entry_id_entry = tk.Entry(window)
min_entry_id_entry.pack()

# Tạo nhãn và ô nhập liệu cho max_entry_id
max_entry_id_label = tk.Label(window, text="Kết thúc:")
max_entry_id_label.pack()
max_entry_id_entry = tk.Entry(window)
max_entry_id_entry.pack()

# Tạo nút "Lọc"
filter_button = tk.Button(window, text="Lọc", command=filter_data)
filter_button.pack()

# Chạy vòng lặp giao diện chính
window.mainloop()
