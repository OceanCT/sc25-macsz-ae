import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
dataset_map = {
    "SDRBENCH-CESM-ATM-26x1800x3600": "CESM-ATM",
    "SDRBENCH-exaalt-copper": "EXAALT",
    "HurricaneISABEL/100x500x500": "Hurricane",
    "HACC/280953867": "HACC",
}


error_bounds = [0.01, 0.001, 0.0001]
colors = ['#4C78A8', '#F58518', '#E45756']  # Blue, Orange, Red
hatches = ['/', '\\', '|']  # Different hatch styles
desired_order = ['orisz', 'adt_fse', 'mac', 'lpaq',  'zfp']
df = pd.read_csv('./datap/pw_rel.csv')

table_rows = []
# print(df)
desired_order = ['orisz', 'adt_fse', 'mac', 'lpaq',  'zfp']
for dataset in list(dataset_map.values())[:4]:
    for error_bound in error_bounds:
        dataset_df = df[(df["Precision"] == error_bound) & (df["Dataset"] == dataset)]
        avg_compressed_sizes = dataset_df.groupby("Method")["CompressedSize"].mean()
        avg_ori_size = dataset_df.groupby("Method")["OriFileSize"].mean()
        avg_compression_ratio = avg_ori_size / avg_compressed_sizes
        avg_compression_ratio = avg_compression_ratio.reindex(desired_order)

        row = {
            "Dataset": dataset,
            "Error Bound": f"{error_bound:.0e}"
        }
        for method in desired_order:
            row[method] = round(avg_compression_ratio.get(method, np.nan), 2)
        table_rows.append(row)

# 转成 DataFrame 并打印
result_df = pd.DataFrame(table_rows)
print(result_df.to_markdown(index=False))


