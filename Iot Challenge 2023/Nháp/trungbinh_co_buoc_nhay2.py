import csv

field1_data = []

# Đọc dữ liệu từ file dulieum.csv
with open('merged_data.csv', 'r') as csvfile:
    reader = csv.DictReader(csvfile)
    for row in reader:
        trung_binh = float(row['field1'])
        field1_data.append(trung_binh)

# Tính trung bình cửa sổ dữ liệu và làm tròn đến chữ số thập phân thứ hai
lay_them = 10
buoc_nhay = 30
do_truot = buoc_nhay + lay_them  
du = (len(field1_data) - lay_them) % buoc_nhay
dulieu_trungbinh = []
print("Số dữ liệu dư là: " + str(du))
for i in range(du, len(field1_data) - du, buoc_nhay):
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
