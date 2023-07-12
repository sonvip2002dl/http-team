import requests
import json
import csv
import time
import glob
import os

def download_data(): 
    url = 'https://api.thingspeak.com/channels/2007956/feeds.json?api_key=NDVML0QGGQINNVYO&results=1'
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

    print(f"Downloaded and saved {len(extracted_data)} data to file {filename}.")

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

    print(f"Merged data saved to file {merged_filename}.")

    # Remove individual CSV files
    for file in csv_files:
        os.remove(file)

while True:
    download_data()
    time.sleep(1)
