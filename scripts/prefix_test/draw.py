import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import matplotlib

# 使用 headless 后端
matplotlib.use('Agg')
colors = ['#4C78A8', '#F58518', '#E45756']

# ==== Step 1: 读入数据 ====
col_names = ['filename', 'opt', 'd1', 'd2', 'd3', 'd4', 'd5', 'd6',
             'orig_size', 'comp_size', 'comp_time', 'other']
df = pd.read_csv('result', header=None, names=col_names)

# ==== Step 2: 构造配置编码（不包含 d5） ====
dims = ['d1', 'd2', 'd3', 'd4', 'd6']
df['config'] = df[dims].astype(str).agg(''.join, axis=1)

# ==== Step 3: 分组计算平均压缩后大小 ====
grouped = df.groupby(['config', 'opt'])['comp_size'].mean().unstack()

# ==== Step 4: 准备横轴 ====
configs = grouped.index.tolist()
x = np.arange(len(configs))
bar_width = 0.35

# ==== Step 5: 开始画图 ====
fig, ax = plt.subplots(figsize=(18, 10))

# 柱状图
bars1 = ax.bar(
    x - bar_width/2, grouped[0], width=bar_width,
    label='General Modeling', color=colors[0], hatch='/', zorder=1
)
bars2 = ax.bar(
    x + bar_width/2, grouped[1], width=bar_width,
    label='Structure-based Modeling', color=colors[1], hatch='\\', zorder=1
)
# ==== Step 7: 美化 ====
ax.tick_params(axis='both', labelsize=30)
ax.yaxis.get_offset_text().set_fontsize(30)
ax.set_xticks(x)
ax.set_xticklabels(configs, rotation=90)
ax.set_xlabel('Enabled Prefixes', fontsize=40)
ax.set_ylabel('Compressed Size', fontsize=40)
ymin = min(grouped.min()) * 0.98
ymax = max(grouped.max()) * 1.02
ax.set_ylim([ymin, ymax])

ax.legend(fontsize=30, ncol=2, loc='upper right')

plt.tight_layout()
plt.grid(True, axis='y', linestyle='--', alpha=0.5)

# ==== Step 8: 保存图像 ====
plt.savefig("prefix.pdf")

