import pandas as pd
import matplotlib.pyplot as plt
df = pd.read_csv("../visualize/datap/pw_rel.csv")
df = df[df["Method"] == "orisz"]
df["CompressedQfsize"] = df["CompressedQfsize"].astype(int)
df["CompressedSize"] = df["CompressedSize"].astype(int)

dataset_map = {
    "SDRBENCH-CESM-ATM-26x1800x3600": "CESM-ATM",
    "SDRBENCH-exaalt-copper": "EXAALT",
    "HurricaneISABEL/100x500x500": "Hurricane",
    "HACC/280953867": "HACC",
}
datasets = df["Dataset"].unique()
for dataset in datasets:
    df1 = df[df["Dataset"] == dataset]
    for precision in [1e-2, 1e-3, 1e-4]:
        df2 = df1[df1["Precision"] == precision]
        # print(df2)
        cqfs = df2.groupby("Method")["CompressedQfsize"].mean()
        cfs = df2.groupby("Method")["CompressedSize"].mean()
        print(dataset, precision ,cqfs["orisz"], cfs["orisz"])
    
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

# data from previous code
data = [
    ["CESM-ATM", 0.01, 91555840.93333334, 94216163.2],
    ["CESM-ATM", 0.001, 144478680.33333334, 147814717.4],
    ["CESM-ATM", 0.0001, 205171000.06666666, 209423231.73333332],
    ["Hurricane", 0.01, 10176890.333333334, 10686967.066666666],
    ["Hurricane", 0.001, 16234408.333333334, 16816970.8],
    ["Hurricane", 0.0001, 26853682.8, 27601075.933333334],
    ["EXAALT", 0.01, 49912194.666666664, 50951213.333333336],
    ["EXAALT", 0.001, 85296461.0, 86272061.33333333],
    ["EXAALT", 0.0001, 123063601.66666667, 124217485.33333333],
    ["HACC", 0.01, 144627201.0, 156464842.33333334],
    ["HACC", 0.001, 228360919.33333334, 243439895.83333334],
    ["HACC", 0.0001, 341977903.0, 355358302.3333333],
]

# 创建 DataFrame
df = pd.DataFrame(data, columns=["Dataset", "Precision", "QfSize", "TotalSize"])
df["Ratio"] = df["QfSize"] / df["TotalSize"]

# 配色和 hatch 图案
colors = ['#4C78A8', '#F58518', '#E45756', '#72B7B2']
hatches = ['/', '\\', 'x', '-']
datasets = ["CESM-ATM", "Hurricane", "EXAALT", "HACC"]
precisions = sorted(df["Precision"].unique(), reverse=True)
bar_width = 0.2
x = np.arange(len(precisions))

# 开始画图
plt.rcParams.update({'font.size': 20})
plt.figure(figsize=(20, 10))

# 绘制每个数据集的柱子
for i, dataset in enumerate(datasets):
    subset = df[df["Dataset"] == dataset].sort_values("Precision", ascending=False)
    bars = plt.bar(x + i * bar_width,
                   subset["Ratio"],
                   width=bar_width,
                   label=dataset,
                   color=colors[i],
                   hatch=hatches[i],
                   edgecolor='black')  # 加黑边以便看清图案

# 设置坐标轴和标签
plt.xticks(x + bar_width * 1.5, [f'{p:.0e}' for p in precisions], fontsize=40)
plt.yticks(fontsize=40)
plt.xlabel("Precision", fontsize=40)
plt.ylabel("Qf Size / Total Size", fontsize=40)
plt.ylim(0.9, 1.0)
plt.legend(fontsize=30, ncol=2)
plt.grid(True, axis='y')
plt.tight_layout()
plt.savefig("./qf_ratio.pdf", bbox_inches='tight')
plt.show()

