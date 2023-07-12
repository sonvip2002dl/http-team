import pandas as pd
import numpy as np
from sklearn.linear_model import LinearRegression
import matplotlib.pyplot as plt
import csv
import time
import urllib2
import json
import glob
import os

url_pm25 = 'https://api.thingspeak.com/update?api_key=PC1RY78Z1CKPTBFR'
first_run = True

def download_data():
    global first_run
    if first_run:
        url = "https://api.thingspeak.com/channels/2007956/feeds.json?api_key=NDVML0QGGQINNVYO&results=100000"
        first_run = False
    else:
        url = "https://api.thingspeak.com/channels/2007956/feeds.json?api_key=NDVML0QGGQINNVYO&results=1"
        
    headers = {'Pragma': 'no-cache'}
    # Send GET request to Thingspeak API
    response = urllib2.urlopen(url)
    data = json.load(response)

    # Extract required fields from the response data
    feeds = data["feeds"]
    fields = ["created_at", "entry_id", "field1", "field2", "field3", "field4", "field5", "field6"]
    extracted_data = [[feed[field] for field in fields] for feed in feeds]

    # Save data to a CSV file
    filename = "data_" + str(time.time()) + ".csv"
    with open(filename, "w") as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(fields)  # Write header
        writer.writerows(extracted_data)

    print "Da tai xuong %d du lieu vao File ===> %s." % (len(extracted_data), filename)

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
    with open(merged_filename, "w") as merged_file:
        writer = csv.writer(merged_file)
        writer.writerow(fields)  # Write header
        writer.writerows(unique_data)

    print "Da luu du lieu vao File ===> %s." % merged_filename

    # Remove individual CSV files
    for file in csv_files:
        os.remove(file)

while True:
    download_data()
    time.sleep(1)
    field1_data = []
    # Doc du lieu tu file merged_data.csv
    with open('merged_data.csv', 'r') as csvfile:
        reader = csv.DictReader(csvfile)
        for row in reader:
            trung_binh = float(row['field1'])
            field1_data.append(trung_binh)

    # Tinh trung binh cua so du lieu va lam tron den chu so thap phan thu hai
    lay_them = 10
    buoc_nhay = 30
    do_truot = buoc_nhay + lay_them  
    du = (len(field1_data) - lay_them) % buoc_nhay
    dulieu_trungbinh = []
    print "So du lieu du la: " + str(du)

    for i in range(du, len(field1_data) - buoc_nhay-1, buoc_nhay):
        cua_so = field1_data[i:i+do_truot] 
        if len(cua_so) == do_truot:
            trung_binh = round(sum(cua_so) / do_truot, 2)
            dulieu_trungbinh.append(trung_binh)

    print "Cua so lay mau la: " + str(len(cua_so))
    time.sleep(1)
    # Luu du lieu vao file CSV neu cua_so co do dai la 40
    if du == 0:
        with open('dulieu_trungbinh.csv', 'w') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow(['Trung_binh'])
            writer.writerows(zip(dulieu_trungbinh))
            print "Dang phan tich..."
        
        # Doc du lieu tu file CSV
        df = pd.read_csv('dulieu_trungbinh.csv')

        # Chuan bi du lieu cho mo hinh
        X = df['Trung_binh'].values[:-1].reshape(-1, 1)
        y = df['Trung_binh'].values[1:]

        # Chia du lieu thanh tap huan luyen va tap kiem tra
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

        # Huan luyen mo hinh hoi quy tuyen tinh
        model = LinearRegression()
        model.fit(X_train, y_train)

        # Du doan tren tap kiem tra va tap val
        y_pred_test = model.predict(X_test)
        y_pred_val = model.predict(X_val)

        # Tinh do loi trung binh binh phuong tren tap kiem tra va tap val
        mse_test = np.mean((y_pred_test - y_test) ** 2)
        mse_val = np.mean((y_pred_val - y_val) ** 2)

        # Tinh RMSE bang cach lay can bac hai cua MSE
        rmse_test = np.sqrt(mse_test)
        rmse_val = np.sqrt(mse_val)

        # In gia tri du doan cuoi cung
        final_prediction = y_pred_test[-1]
        print "Gia tri du doan: %s" % final_prediction

        # Doc du lieu tu file CSV
        df = pd.read_csv('merged_data.csv')

        # Lay 30 va 10 dong gan nhat
        recent_data30 = df.tail(30)
        recent_data10 = df.tail(10)
        # Tinh tong cua field1
        total_field1 = recent_data30['field1'].sum()
        total_field2 = recent_data10['field1'].sum()
        gia_tri_thuc = (final_prediction*40 - total_field2)/30
        print "Tong cua 30 du lieu gan nhat la:", total_field1
        print "Tong cua 10 du lieu gan nhat la:", total_field2
        print "Gia tri 10 phut toi: ", gia_tri_thuc
        print "Do loi trung binh tren tap kiem tra: %.2f" % rmse_test
        print "Do loi trung binh binh phuong tren tap kiem tra: %.2f" % mse_test
        requests.get(url_pm25 + '&field1' + '=' + str(gia_tri_thuc) + '&field2' + '='+ str(rmse_test))
        print "Da gui du lieu len Thingspeak"
        print "===================================================="
        time.sleep(25)
