import pandas as pd
import matplotlib.pyplot as plt


# gradient_colors = [
#     '#008B8B',   
#     '#20B2AA',    
#     '#1E90FF',  
#     '#0000CD',  
# ]
#2CA02C
gradient_colors = ['#4C78A8', '#F58518', '#E45756', '#2CA02C']
# plt.rcParams['font.family'] = ['Times new roman']
plt.rcParams['font.size'] = '18'
# plt.rcParams['font.sans-serif'] = ['Times new roman']

hatch_patterns = ['/', '\\', '/', '\\']

error_bound_map = {
    "sz2": "1e-2",
    "sz3": "1e-3",
    "sz4": "1e-4",
}

def plot_compression_ratios(csv_file):
    data = pd.read_csv(csv_file)

    # 遍历每个文件的记录
    for file_name in data['file'].unique():
        # 获取当前文件的数据
        file_data = data[data['file'] == file_name]
        dataset = file_data['dataset'].values[0]
        
        # 初始化图表
        plt.figure(figsize=(6, 6))
        
        x_labels = list(error_bound_map.values())  # 横坐标：不同的 error_bound
        x = range(len(x_labels))
        width = 0.18  # 每组柱子的宽度变细

        # 存储每种方法的压缩比
        compression_data = {
            'Huff': [],
            'Huff+LZ4': [],
            'LZ4': [],
            'LZ4+Huff': []
        }


        # 遍历每个 error_bound 的记录
        for error_bound in file_data['error_bound'].unique():
            # 获取当前 error_bound 的数据
            error_data = file_data[file_data['error_bound'] == error_bound]

            # 提取需要的列
            data_size = error_data['data_size'].values[0]
            huff_size = error_data['huff_size'].values[0]
            huff_lz4_size = error_data['huff_lz4_size'].values[0]
            lz4_size = error_data['lz4_size'].values[0]
            lz4_huff_size = error_data['lz4_huff_size'].values[0]

            # 计算压缩比
            compression_ratios = [
                data_size / huff_size,
                data_size / huff_lz4_size,
                data_size / lz4_size,
                data_size / lz4_huff_size,
            ]

            # 将每个压缩比加入对应方法的列表
            compression_data['Huff'].append(compression_ratios[0])
            compression_data['Huff+LZ4'].append(compression_ratios[1])
            compression_data['LZ4'].append(compression_ratios[2])
            compression_data['LZ4+Huff'].append(compression_ratios[3])
            print(file_name)
            print(error_bound)
            print(compression_ratios)
        # 在柱状图上绘制数据
        for i, method in enumerate(compression_data):
            # 设置颜色和斜杠样式
            color = gradient_colors[i % len(gradient_colors)]
            hatch = hatch_patterns[i % len(hatch_patterns)]
            
            plt.bar(
                [pos + i * width for pos in x], 
                compression_data[method], 
                width=width,  # 更细的柱子
                color=color,  # 设置渐变红色
                hatch=hatch,  # 设置不同的斜杠填充
                label=method
            )

        # 图表设置
        plt.title(f'{dataset}', fontsize=18, fontweight='bold')
        plt.xlabel('Error Bound', fontsize=18, fontweight='bold')
        plt.ylabel('Compression Ratio', fontsize=18, fontweight='bold')
        plt.xticks([pos + (len(file_data['error_bound'].unique()) - 1) * width / 2 for pos in x], x_labels, fontsize=18, fontweight='bold')
        plt.yticks(fontsize=18, fontweight='bold')
        plt.ylim(0, 11)
        plt.grid(axis='y', linestyle='--', alpha=0.6)
        plt.legend(fontsize=12)

        # 保存或显示图表
        plt.tight_layout()
        plt.savefig(f'./{dataset}_compression_ratios.pdf')
        plt.close()

# 调用函数
csv_file = './input.csv'  # 替换为你的CSV文件路径
plot_compression_ratios(csv_file)

