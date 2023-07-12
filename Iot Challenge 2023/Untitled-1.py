import requests
import pandas as pd

# Lấy dữ liệu từ API
url = "https://api.thingspeak.com/channels/2007956/feeds.json?results=10000000000"
response = requests.get(url)
data = response.json()

# Chuyển dữ liệu thành DataFrame của Pandas
df = pd.DataFrame(data["feeds"])

# Chuyển kiểu dữ liệu của cột "field1" và "field2" sang số
df["field1"] = pd.to_numeric(df["field1"])
df["field2"] = pd.to_numeric(df["field2"])

# Tính trung bình của các giá trị trong 1 giờ
df["created_at"] = pd.to_datetime(df["created_at"])
df = df.set_index("created_at").resample("1H").mean().reset_index()

# In ra 5 dòng đầu tiên của DataFrame
print(df.head())
