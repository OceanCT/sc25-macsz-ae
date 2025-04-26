import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
file_path = "../pw_rel_test/result"
dataset_map = {
    "SDRBENCH-CESM-ATM-26x1800x3600": "CESM-ATM",
    "SDRBENCH-exaalt-copper": "EXAALT",
    "HurricaneISABEL/100x500x500": "Hurricane",
    "HACC/280953867": "HACC",
}

df = pd.read_csv(file_path)
df.columns = [
    'FileName', 'Method', 'Precision', 'OriFileSize', 'CompressedSize',
    'CompressionRatio', 'Qfsize', 'CompressedQfsize',
    'Compressiontime', 'Decompressiontime'
]
df = df.sort_values(by=["FileName", "Precision", "Method"], ascending=True)
def get_dataset(row):
    for key in dataset_map.keys():
        if key in row["FileName"]:
            return dataset_map[key]
    return "Unknown"

df["Dataset"] = df.apply(get_dataset, axis=1)
df.to_csv("./datap/pw_rel.csv", index=False)
print(df.head())
