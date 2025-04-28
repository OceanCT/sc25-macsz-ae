import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.cm as cm

from collections import Counter, defaultdict
import struct
import numpy as np
import time
import pickle
import os

# ========== 文件路径 ==========
filenames = [
    # "../CESM-ATM/szdats/CLDLIQ_1_26_1800_3600.f32.sz2.szdat",
    # "../CESM-ATM/szdats/CLDLIQ_1_26_1800_3600.f32.sz3.szdat",
    # "../CESM-ATM/szdats/CLDLIQ_1_26_1800_3600.f32.sz4.szdat",
    # "../EXAALT/szdats/dataset2-83x1077290.x.f32.dat.sz2.szdat",
    # "../EXAALT/szdats/dataset2-83x1077290.x.f32.dat.sz3.szdat",
    # "../EXAALT/szdats/dataset2-83x1077290.x.f32.dat.sz4.szdat",
    # "../HACC/szdats/vx.f32.sz2.szdat",
    # "../HACC/szdats/vx.f32.sz3.szdat",
    # "../HACC/szdats/vx.f32.sz4.szdat",
    # "../HurricaneISABEL/szdats/Uf48.bin.f32.sz2.szdat",
    # "../HurricaneISABEL/szdats/Uf48.bin.f32.sz3.szdat",
    # "../HurricaneISABEL/szdats/Uf48.bin.f32.sz4.szdat",
    # "../EXAALT_SMALL/szdats/xx.dat2.sz2.szdat",
    # "../EXAALT_SMALL/szdats/xx.dat2.sz3.szdat",
    # "../EXAALT_SMALL/szdats/xx.dat2.sz4.szdat",
]

labels = ["sz2", "sz3", "sz4"]  # 用作图例
colors = [cm.get_cmap('tab10')(i) for i in range(len(filenames))]

bin_size = 5
max_val_limit = 10000
cache_file = "small_exaalt_cache1.pkl"

def read_file_fast(filename):
    counter = Counter()
    chunk_size = 65536
    with open(filename, "rb") as f:
        while True:
            chunk = f.read(chunk_size)
            if not chunk:
                break
            num_values = len(chunk) // 4
            values = struct.unpack(f'<{num_values}I', chunk)
            counter.update(values)
    return counter

start_time = time.time()

# ==== 缓存逻辑 ====
if os.path.exists(cache_file):
    print("Loading cached counters...")
    with open(cache_file, 'rb') as f:
        all_counters, max_values = pickle.load(f)
else:
    all_counters, max_values = [], []
    for filename in filenames:
        print(f"Processing {filename}...")
        counter = read_file_fast(filename)
        all_counters.append(counter)
        max_values.append(max(counter.keys()) if counter else 0)
    with open(cache_file, 'wb') as f:
        pickle.dump((all_counters, max_values), f)

print(f"Data loading took {time.time() - start_time:.2f} seconds")

# ==== 分 bin 和归一化 ====
max_val = min(max(max_values), max_val_limit)
bins = np.arange(0, max_val + bin_size, bin_size)
all_values = bins[:-1]

total_counts = [sum(c.values()) for c in all_counters]
y_data = np.zeros((len(filenames), len(all_values)), dtype=np.float64)

for i, (counter, total) in enumerate(zip(all_counters, total_counts)):
    for val, count in counter.items():
        if val > max_val:
            continue
        bin_idx = min(val // bin_size, len(all_values) - 1)
        y_data[i, bin_idx] += count / total

# ==== 画柱状图 ====
plt.figure(figsize=(20, 8))
bar_width = 0.8 / len(filenames)

for i in range(len(filenames)):
    x_pos = [x + i * bar_width for x in range(len(all_values))]
    plt.bar(x_pos, y_data[i], width=bar_width, color=colors[i], label=labels[i], alpha=0.9)

# ==== 设置轴与图例 ====
plt.xlabel("Quantization factor Value", fontsize=30)
plt.ylabel("Proportion", fontsize=30)
plt.xticks([len(all_values) - 1], [str(max_val)], fontsize=25)
plt.yticks(fontsize=25)
plt.legend(loc='upper right', fontsize=25)
plt.title("Quantization Distribution - EXAALT_SMALL vx.dat2", fontsize=26)
plt.tight_layout()
plt.savefig("exaalt_small_bar.pdf", bbox_inches='tight')
plt.close()

print(f"Total time: {time.time() - start_time:.2f} seconds")