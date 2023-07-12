import requests
import json
import datetime
import schedule
import time
import csv

def download_data():
    # URL API Thingspeak
    url = "https://api.thingspeak.com/channels/2007956/feeds.json?api_key=NDVML0QGGQINNVYO&results=10000"

    # Gửi yêu cầu GET đến Thingspeak API
    response = requests.get(url)
    data = json.loads(response.text)

    # Lưu trữ dữ liệu vào file CSV
    filename = f"dulieum_{datetime.datetime.now().strftime('%Y-%m-%d_%H-%M-%S')}.csv"
    with open(filename, "w", newline="", encoding="utf-8") as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["Trung_binh"])  # Ghi tiêu đề cột
        for entry in data["feeds"]:
            writer.writerow([entry["field1"]])

    print(f"Đã tải xuống và lưu dữ liệu vào file {filename}.")

# Thiết lập lịch trình cho việc tải xuống dữ liệu sau mỗi 30 giây
schedule.every(30).seconds.do(download_data)

while True:
    schedule.run_pending()
    time.sleep(1)
