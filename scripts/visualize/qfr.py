import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.ticker import ScalarFormatter
import numpy as np

# 加载数据
target_eb = 0.01
df = pd.read_csv('./datap/pw_rel.csv')
df = df[df["Dataset"] == "CESM-ATM"]
df = df[df["Precision"] == target_eb]
df = df[df["Method"] != "zfp"]
df['CompressedQfsize'] = df['CompressedQfsize'].astype(int)
df['Qfsize'] = df['Qfsize'].astype(int)
df['FileName'] = df['FileName'].apply(lambda x: x.split("/")[-1])
df['FileName'] = df['FileName'].apply(lambda x: x.split("_")[0])

colors = ['#4C78A8', '#72B7B2', '#F58518', '#E45756', ]  # 蓝色、橙色、红色、绿色
hatches = ['/', '\\', '-', '*']  # 不同的填充图案
desired_order = ['huffman', 'adt-fse', 'mac', 'lpaq']  # 方法的顺序

datasets = df['Dataset'].unique()
df['Method'] = df['Method'].replace({'orisz': 'huffman', 'adt_fse': 'adt-fse'})

for dataset in datasets:
    dataset_df = df[df['Dataset'] == dataset]
    unique_file_names = sorted(dataset_df['FileName'].unique())
    
    n = len(unique_file_names)
    x_positions = np.arange(n)
    m = len(desired_order)
    bar_width = 0.8 / m  # 动态设置柱子宽度以适配方法数量

    plt.figure(figsize=(18, 6))

    for i, method in enumerate(desired_order):
        method_df = dataset_df[dataset_df['Method'] == method]
        ordered_df = pd.DataFrame({'FileName': unique_file_names}).merge(method_df, on='FileName', how='left')
        values = ordered_df['CompressedQfsize'].fillna(0).values
        positions = x_positions + (i - m/2 + 0.5) * bar_width  # 居中对齐
        print(f"Dataset: {dataset}, Method: {method}, Values: {values}")
        plt.bar(positions, values, width=bar_width, color=colors[i], hatch=hatches[i], label=method)

    plt.yticks(fontsize=30, fontweight='bold')
    plt.xticks(x_positions, unique_file_names, rotation=30, fontsize=30, fontweight='bold')
    plt.xlabel('FileName', fontsize=30, fontweight='bold')
    plt.ylabel('Qf Size', fontsize=30, fontweight='bold')

    plt.legend(fontsize=30, ncol=4, loc='lower center', bbox_to_anchor=(0.5, 1.02), frameon=False)
    plt.tight_layout(rect=[0, 0, 1, 0.92])  # 为 legend 留出顶部空间
    plt.gca().yaxis.set_major_formatter(ScalarFormatter())
    plt.gca().yaxis.offsetText.set_fontsize(30)
    plt.gca().yaxis.offsetText.set_fontweight('bold')
    plt.tight_layout()
    plt.grid(axis='y', linestyle='--', alpha=0.7)
    plt.savefig(f"./pics/{dataset}_{target_eb}_qfsize.pdf")

