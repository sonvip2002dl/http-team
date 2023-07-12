import requests
import json
import csv

# Chọn số lượng kết quả cần tải xuống
results = 1000

# Tạo URL để tải dữ liệu từ ThingSpeak
url = f"https://api.thingspeak.com/channels/2129409/feeds.json?results={results}"

# Tải dữ liệu từ ThingSpeak
response = requests.get(url)
data = json.loads(response.text)

# Tạo file CSV và lưu dữ liệu vào đó
with open("data.csv", mode="w", newline="") as file:
    writer = csv.writer(file)
    # Viết tiêu đề cột
    writer.writerow(["timestamp", "value"])
    # Viết dữ liệu
    for feed in data["feeds"]:
        writer.writerow([feed["created_at"], feed["field1"]])
