import os
import re
import itertools
# bitwidth: 9
# original file_size: 11477760,Compressed length: 2879678,total bit num: 25824960,successful predict bit num: 0,bitwidth: 9,time cost: 6.000000

files = [
        #("~/data_pre/EXAALT_SMALL/szdats/vx.dat2.sz2.szdat", 2869440),
        ("./DCQ_1_26_1800_3600.f32.sz2.szdat", 26*1800*3600)
         ]

positions = [1, 2, 3, 4, 6]

# 所有可能的0/1组合
combinations = list(itertools.product([0, 1], repeat=len(positions)))

# 生成所有可能的prefix列表
prefixs = []
for combo in combinations:
    if all(v == 0 for v in combo):
        continue
    prefix = [0] * 7
    for pos, val in zip(positions, combo):
        prefix[pos] = val
    prefixs.append(prefix)
print(prefixs)

def test_file(file_path, size, bit_width_enable, prefix):
    bitwidth_re = r'bitwidth: (.*),'
    qf_size_compressed_re = r'Compressed length: (.*),total'
    ctime_re = r'time cost: (.*)\n'
    cmd = "./main {} {} {} {} {} {} {} {} {}".format(file_path, size, bit_width_enable, prefix[1], prefix[2], prefix[3], prefix[4], prefix[5], prefix[6])
    print(cmd)
    result = os.popen(cmd).read()
    print(result)
    bitwidth = re.findall(bitwidth_re, result)[0]
    qf_size_compressed = re.findall(qf_size_compressed_re, result)[0]
    time = re.findall(ctime_re, result)[0]
    result_str = "{},{},{},{},{},{},{},{},{},{},{},{}".format(file_path, bit_width_enable, prefix[1], prefix[2], prefix[3], prefix[4], prefix[5], prefix[6], size, qf_size_compressed, time, bitwidth)
    print(result_str)
    os.system("echo '{}' >> result".format(result_str))


for file_path, size in files:
    for bit_width_enbale in [0, 1]:
        for prefix in prefixs:
            
            test_file(file_path, size, bit_width_enbale, prefix)

