import requests
import json
import datetime
import schedule
import time
import os
import pandas as pd

def download_data():
    # URL API Thingspeak
    url = "https://api.thingspeak.com/channels/2007956/feeds.json?api_key=NDVML0QGGQINNVYO&results=8000"

    # Gửi yêu cầu GET đến Thingspeak API
    response = requests.get(url)
    data = json.loads(response.text)

    # Lưu trữ dữ liệu vào file JSON
    filename = f"dulieum_{datetime.datetime.now().strftime('%Y-%m-%d_%H-%M-%S')}.json"
    with open(filename, "w") as jsonfile:
        json.dump(data, jsonfile)

    print(f"Đã tải xuống và lưu dữ liệu vào file {filename}.")

    # Gộp và loại bỏ các giá trị trùng lặp
    merge_files()

def merge_files():
    json_files = [file for file in os.listdir() if file.startswith("dulieum_") and file.endswith(".json")]

    if len(json_files) <= 1:
        return

    dfs = []
    for file in json_files:
        try:
            with open(file) as jsonfile:
                data = json.load(jsonfile)
                feeds = data["feeds"]
                for feed in feeds:
                    dfs.append(pd.DataFrame(feed, index=[0]))
        except json.decoder.JSONDecodeError:
            continue

    merged_df = pd.concat(dfs)

    if os.path.exists("merged_data.json"):
        previous_data = pd.read_json("merged_data.json")
        merged_df = pd.concat([previous_data, merged_df])

    merged_df.drop_duplicates(subset=["created_at", "entry_id"], inplace=True)
    merged_df.reset_index(drop=True, inplace=True)  # Xóa thuộc tính index

    merged_filename = "merged_data.json"
    merged_df.to_json(merged_filename, orient="records")

    print(f"Đã gộp và lưu dữ liệu vào file {merged_filename}.")

    # Lọc tệp merged_data.json để loại bỏ các giá trị trùng lặp
    merged_data = pd.read_json(merged_filename)
    merged_data.drop_duplicates(subset=["created_at", "entry_id"], inplace=True)
    merged_data.reset_index(drop=True, inplace=True)
    merged_data.to_json(merged_filename, orient="records")

    # ...
    merged_filename = "merged_data.json"
    merged_data = pd.read_json(merged_filename)
    sorted_data = merged_data.sort_values(by="created_at")

    csv_filename = "data.csv"
    sorted_data.to_csv(csv_filename, index=False)

    print(f"Đã lưu dữ liệu vào file {csv_filename}.")


# Thiết lập lịch trình cho việc tải xuống dữ liệu sau mỗi 30 giây
schedule.every(30).seconds.do(download_data)

while True:
    schedule.run_pending()
    time.sleep(1)
