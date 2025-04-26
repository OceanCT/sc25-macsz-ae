import struct
import os
import pickle
from collections import Counter
import numpy as np

# ==== 文件路径 ====
filenames = [
    "./CLDLIQ_1_26_1800_3600.f32.sz2.szdat",
    "./CLDLIQ_1_26_1800_3600.f32.sz3.szdat",
    "./CLDLIQ_1_26_1800_3600.f32.sz4.szdat",
    "./dataset2-83x1077290.x.f32.dat.sz2.szdat",
    "./dataset2-83x1077290.x.f32.dat.sz3.szdat",
    "./dataset2-83x1077290.x.f32.dat.sz4.szdat",
    "./vx.f32.sz2.szdat",
    "./vx.f32.sz3.szdat",
    "./vx.f32.sz4.szdat",
    "./Uf48.bin.f32.sz2.szdat",
    "./Uf48.bin.f32.sz3.szdat",
    "./Uf48.bin.f32.sz4.szdat",
]

# ==== 缓存文件 ====
cache_file = "counters_cache.pkl"

# ==== 快速读取文件 ====
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

# ==== 加载缓存或重新读取 ====
if os.path.exists(cache_file):
    with open(cache_file, 'rb') as f:
        all_counters, _ = pickle.load(f)
else:
    all_counters = []
    for filename in filenames:
        print(f"Reading {filename} ...")
        counter = read_file_fast(filename)
        all_counters.append(counter)
    with open(cache_file, 'wb') as f:
        pickle.dump((all_counters, [max(c.keys()) for c in all_counters]), f)

# ==== 统计每个文件的90%区间和最大量化因子 ====
for filename, counter in zip(filenames, all_counters):
    # 排除零值
    counter = {key: count for key, count in counter.items() if key != 0}
    
    total = sum(counter.values())
    if total == 0:
        print(f"{filename}: empty or corrupted (no non-zero quantization factors).")
        continue

    # 找出现频率最高的 bin，跳过0值
    center = max(counter.items(), key=lambda x: x[1])[0]

    # 构造对称扩展
    coverage = counter[center] / total
    offset = 1
    left, right = center, center

    while coverage < 0.9:
        left_val = counter.get(center - offset, 0)
        right_val = counter.get(center + offset, 0)
        coverage += (left_val + right_val) / total
        if left_val > 0:
            left = center - offset
        if right_val > 0:
            right = center + offset
        offset += 1
        if left == 0 and right == max(counter.keys()):
            break  # 已经遍历完所有值

    max_val = max(counter.keys())
    label = filename
    print(f"{label}: max_val={max_val}, 90% symmetric range = [{left}, {right}] centered at {center}")
