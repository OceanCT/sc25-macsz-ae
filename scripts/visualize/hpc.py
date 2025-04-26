import matplotlib.pyplot as plt
from matplotlib.patches import Patch
import numpy as np

# 原始数据
data = {
    "lpaq-sz": {
        "1024": [364, 55],
        "2048": [365, 110],
        "4096": [366, 220],
        "8192": [373, 443],
    },
    "mac-sz": {
        "1024": [88, 53],
        "2048": [94, 107],
        "4096": [82, 215],
        "8192": [93, 433],
    },
    "orisz": {
        "1024": [42, 62],
        "2048": [46, 124],
        "4096": [52, 249],
        "8192": [52, 501],
    },
    "adt-fse-sz": {
        "1024": [28, 68],
        "2048": [31, 137],
        "4096": [36, 276],
        "8192": [36, 554],
    },
    "zfp": {
        "1024": [6, 406],
        "2048": [6, 812],
        "4096": [6, 1625],
        "8192": [6, 3250],
    },
    "none": {
        "1024": [0, 790],
        "2048": [0, 1580],
        "4096": [0, 3160],
        "8192": [0, 6320],
    }
}


methods = ["orisz", "adt-fse-sz", "mac-sz", "lpaq-sz",  "zfp", "none"]
cores = ["1024", "2048", "4096", "8192"]

compression_color = '#F58518'
write_color = '#E45756'
compression_hatch = '/'
write_hatch = '\\'

plt.rcParams.update({
    'font.size': 18,
    'font.weight': 'bold',
    'axes.labelweight': 'bold',
})

fig, axes = plt.subplots(2, len(cores), figsize=(12, 6), sharex='col', sharey='row',
                         gridspec_kw={'height_ratios': [1, 2]})
fig.subplots_adjust(hspace=0.05, wspace=0.05)

bar_width = 0.6
y_break = 1000

for idx, core in enumerate(cores):
    ax_upper = axes[0, idx]
    ax_lower = axes[1, idx]
    x = np.arange(len(methods))
    
    for i, method in enumerate(methods):
        t1, t2 = data[method][core]
        total = t1 + t2
        
        if total > y_break:
            if t1 > 0:
                ax_lower.bar(x[i], t1, color=compression_color, hatch=compression_hatch,
                             width=bar_width, edgecolor='black', linewidth=0.3)
                ax_lower.bar(x[i], y_break - t1, bottom=t1, color=write_color, hatch=write_hatch,
                             width=bar_width, edgecolor='black', linewidth=0.3)
                ax_upper.bar(x[i], total - y_break, color=write_color, hatch=write_hatch,
                             width=bar_width, edgecolor='black', linewidth=0.3)
            else:
                # ✅ 只有读取时间：全部画红色
                ax_lower.bar(x[i], y_break, color=write_color, hatch=write_hatch,
                             width=bar_width, edgecolor='black', linewidth=0.3)
                ax_upper.bar(x[i], total - y_break, color=write_color, hatch=write_hatch,
                             width=bar_width, edgecolor='black', linewidth=0.3)
        else:
            if t1 > 0:
                ax_lower.bar(x[i], t1, color=compression_color, hatch=compression_hatch,
                             width=bar_width, edgecolor='black', linewidth=0.3)
                ax_lower.bar(x[i], t2, bottom=t1, color=write_color, hatch=write_hatch,
                             width=bar_width, edgecolor='black', linewidth=0.3)
            else:
                ax_lower.bar(x[i], t2, color=write_color, hatch=write_hatch,
                             width=bar_width, edgecolor='black', linewidth=0.3)

    ax_lower.set_xticks(x)
    ax_lower.set_xticklabels(methods, fontsize=20, rotation=80)
    # ax_upper.set_xticks(x)
    # ax_upper.set_xticklabels([""]*len(methods))
    ax_upper.set_title(f"{core} cores", fontsize=20, fontweight='bold')

# y 轴设置
axes[0, 0].set_ylim(500, 7000)
axes[1, 0].set_ylim(0, y_break)

# 波浪断裂标记
d = .5
kwargs = dict(marker=[(-1, -d), (1, d)], markersize=12,
              linestyle="none", color='k', mec='k', mew=1, clip_on=False)
for idx in range(len(cores)):
    axes[0, idx].plot([0, 1], [0, 0], transform=axes[0, idx].transAxes, **kwargs)
    axes[1, idx].plot([0, 1], [1, 1], transform=axes[1, idx].transAxes, **kwargs)

# y 轴标签和图例
for ax in axes[1, :]:
    ax.grid(True, axis='y', linestyle='--', linewidth=0.5, alpha=0.6)
axes[1, 0].set_ylabel("Time (s)", fontsize=16, fontweight='bold')

legend_elements = [
    Patch(facecolor=compression_color, hatch=compression_hatch, edgecolor='black', label='Compression Time'),
    Patch(facecolor=write_color, hatch=write_hatch, edgecolor='black', label='Writing Time'),
]


plt.tight_layout()
fig.legend(handles=legend_elements,
           loc='upper center',
           bbox_to_anchor=(0.5, 1.05),
           fontsize=20,
           ncol=2,
           frameon=False)
plt.savefig("./pics/hpc/dumping.pdf",bbox_inches='tight')
import matplotlib.pyplot as plt
from matplotlib.patches import Patch
import matplotlib.patches as mpatches
import numpy as np

# 原始数据
# data = {
#     "lpaq": {
#         "1024": [364, 55],
#         "2048": [365, 109],
#         "4096": [366, 220],
#         "8192": [373, 443],
#     },
#     "mac": {
#         "1024": [88, 53],
#         "2048": [94, 107],
#         "4096": [82, 215],
#         "8192": [93, 433],
#     },
#     "orisz": {
#         "1024": [42, 62],
#         "2048": [46, 124],
#         "4096": [52, 249],
#         "8192": [52, 501],
#     },
#     "zfp": {
#         "1024": [6, 406],
#         "2048": [6, 812],
#         "4096": [6, 1625],
#         "8192": [6, 3250],
#     },
#     "none": {
#         "1024": [0, 790],
#         "2048": [0, 1580],
#         "4096": [0, 3160],
#         "8192": [0, 6320],
#     }
# }
data = {
    "lpaq-sz": {
        "1024": [356, 78],
        "2048": [354, 157],
        "4096": [355, 315],
        "8192": [362, 631],
    },
    "mac-sz": {
        "1024": [76, 76],
        "2048": [76, 153],
        "4096": [78, 307],
        "8192": [78, 615],
    },
    "orisz": {
        "1024": [4, 98],
        "2048": [4, 197],
        "4096": [6, 356],
        "8192": [6, 712],
    },
    "adt-fse-sz": {
        "1024": [28, 68],
        "2048": [31, 137],
        "4096": [5, 394],
        "8192": [6, 788],
    },
    "zfp": {
        "1024": [4, 406],
        "2048": [4, 812],
        "4096": [4, 1625],
        "8192": [4, 3250],
    },
    "none": {
        "1024": [0, 790],
        "2048": [0, 1580],
        "4096": [0, 3160],
        "8192": [0, 6320],
    }
}
methods = ["orisz", "adt-fse-sz", "mac-sz", "lpaq-sz",  "zfp", "none"]
cores = ["1024", "2048", "4096", "8192"]

compression_color = '#F58518'
write_color = '#E45756'
compression_hatch = '/'
write_hatch = '\\'

plt.rcParams.update({
    'font.size': 18,
    'font.weight': 'bold',
    'axes.labelweight': 'bold',
})

fig, axes = plt.subplots(2, len(cores), figsize=(12, 6), sharex='col', sharey='row',
                         gridspec_kw={'height_ratios': [1, 2]})
fig.subplots_adjust(hspace=0.05, wspace=0.05)

bar_width = 0.6
y_break = 1000

for idx, core in enumerate(cores):
    ax_upper = axes[0, idx]
    ax_lower = axes[1, idx]
    x = np.arange(len(methods))
    
    for i, method in enumerate(methods):
        t1, t2 = data[method][core]
        total = t1 + t2
        
        if total > y_break:
            if t1 > 0:
                ax_lower.bar(x[i], t1, color=compression_color, hatch=compression_hatch,
                             width=bar_width, edgecolor='black', linewidth=0.3)
                ax_lower.bar(x[i], y_break - t1, bottom=t1, color=write_color, hatch=write_hatch,
                             width=bar_width, edgecolor='black', linewidth=0.3)
                ax_upper.bar(x[i], total - y_break, color=write_color, hatch=write_hatch,
                             width=bar_width, edgecolor='black', linewidth=0.3)
            else:
                # ✅ 只有读取时间：全部画红色
                ax_lower.bar(x[i], y_break, color=write_color, hatch=write_hatch,
                             width=bar_width, edgecolor='black', linewidth=0.3)
                ax_upper.bar(x[i], total - y_break, color=write_color, hatch=write_hatch,
                             width=bar_width, edgecolor='black', linewidth=0.3)
        else:
            if t1 > 0:
                ax_lower.bar(x[i], t1, color=compression_color, hatch=compression_hatch,
                             width=bar_width, edgecolor='black', linewidth=0.3)
                ax_lower.bar(x[i], t2, bottom=t1, color=write_color, hatch=write_hatch,
                             width=bar_width, edgecolor='black', linewidth=0.3)
            else:
                ax_lower.bar(x[i], t2, color=write_color, hatch=write_hatch,
                             width=bar_width, edgecolor='black', linewidth=0.3)

    ax_lower.set_xticks(x)
    ax_lower.set_xticklabels(methods, fontsize=20, rotation=80)
    # ax_upper.set_xticks(x)
    # ax_upper.set_xticklabels([""]*len(methods))
    ax_upper.set_title(f"{core} cores", fontsize=20, fontweight='bold')

# y 轴设置
axes[0, 0].set_ylim(500, 7000)
axes[1, 0].set_ylim(0, y_break)

# 波浪断裂标记
d = .5
kwargs = dict(marker=[(-1, -d), (1, d)], markersize=12,
              linestyle="none", color='k', mec='k', mew=1, clip_on=False)
for idx in range(len(cores)):
    axes[0, idx].plot([0, 1], [0, 0], transform=axes[0, idx].transAxes, **kwargs)
    axes[1, idx].plot([0, 1], [1, 1], transform=axes[1, idx].transAxes, **kwargs)

# y 轴标签和图例
for ax in axes[1, :]:
    ax.grid(True, axis='y', linestyle='--', linewidth=0.5, alpha=0.6)
axes[1, 0].set_ylabel("Time (s)", fontsize=20)

legend_elements = [
    Patch(facecolor=compression_color, hatch=compression_hatch, edgecolor='black', label='Decompression Time'),
    Patch(facecolor=write_color, hatch=write_hatch, edgecolor='black', label='Reading Time'),
]


plt.tight_layout()
fig.legend(handles=legend_elements,
           loc='upper center',
           bbox_to_anchor=(0.5, 1.05),
           fontsize=20,
           ncol=2,
           frameon=False)
plt.savefig("./pics/hpc/loading.pdf", bbox_inches='tight')
import matplotlib.pyplot as plt
import numpy as np

compression_ratio = {
    "lpaq-sz": 9.6,
    "mac-sz": 9.8,
    "orisz": 8.4,
    "adt-fse-sz": 7.66,
    "zfp": 2.2,
}

ori_size = 673_920_000  # 原始数据大小，单位字节
compression_time = {
    "lpaq-sz": 373 + 443,
    "mac-sz": 93 + 433,
    "orisz": 52 + 501,
    "zfp": 6 + 3250,
    "adt-fse-sz": 36 + 554,
}



# 计算吞吐量
throughput = {k: ori_size / compression_time[k] for k in compression_ratio}
x = [throughput[key] / 1e6 for key in compression_ratio]  # 转换为 MB/s
y = [compression_ratio[key] for key in compression_ratio]
labels = list(compression_ratio.keys())
print(throughput)
colors = {
    "lpaq-sz": "red",
    "mac-sz": "green",
    "orisz": "blue",
    "adt-fse-sz": "orange",
    "zfp": "purple",
}
markers = {
    "lpaq-sz": "o",    # 圆圈
    "mac-sz": "s",     # 方块
    "orisz": "^",   # 三角形
    "zfp": "D",     # 菱形
    "adt-fse-sz": "X",  # X形
}

plt.figure(figsize=(9, 6))

for label in labels:
    plt.scatter(throughput[label] / 1e6, compression_ratio[label],
                color=colors[label], marker=markers[label],
                label=label, s=100)

plt.xlabel("Normalized dumping throughput", fontsize=30)
plt.ylabel("Compression ratio", fontsize=30)
plt.legend(fontsize=28)
plt.tick_params(labelsize=30)
plt.grid(True)
plt.tight_layout()
# plt.show()
plt.savefig("./pics/hpc/cluster_cmp_8192_dump.pdf")
import matplotlib.pyplot as plt
import numpy as np

compression_ratio = {
    "lpaq-sz": 9.6,
    "mac-sz": 9.8,
    "orisz": 8.4,
    "adt-fse-sz": 7.66,
    "zfp": 2.2,
}

ori_size = 673_920_000  # 原始数据大小，单位字节

compression_time = {
    "lpaq-sz": 362 + 631,
    "mac-sz": 74 + 616,
    "orisz": 6 + 712,
    "zfp": 6 + 3270,
    "adt-fse-sz": 6 + 788,
}

# 计算吞吐量
throughput = {k: ori_size / compression_time[k] for k in compression_ratio}
x = [throughput[key] / 1e6 for key in compression_ratio]  # 转换为 MB/s
y = [compression_ratio[key] for key in compression_ratio]
labels = list(compression_ratio.keys())
print(throughput)
colors = {
    "lpaq-sz": "red",
    "mac-sz": "green",
    "orisz": "blue",
    "zfp": "purple",
    "adt-fse-sz": "orange",
}
markers = {
    "lpaq-sz": "o",    # 圆圈
    "mac-sz": "s",     # 方块
    "orisz": "^",   # 三角形
    "zfp": "D",     # 菱形
    "adt-fse-sz": "X",  # X形
}


plt.figure(figsize=(9, 6))

for label in labels:
    plt.scatter(throughput[label] / 1e6, compression_ratio[label],
                color=colors[label], marker=markers[label],
                label=label, s=100)

plt.xlabel("Normalized loading throughput", fontsize=30)
plt.ylabel("Compression ratio", fontsize=30)
plt.legend(fontsize=28)
plt.tick_params(labelsize=30)
plt.grid(True)
plt.tight_layout()
# plt.show()
plt.savefig("./pics/hpc/cluster_cmp_8192_load.pdf")



