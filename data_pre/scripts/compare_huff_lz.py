import os
target_dirs = ["CESM-ATM", "EXAALT", "HurricaneISABEL","HACC"] 
target_datas = ["CLDLIQ_1_26_1800_3600.f32","dataset2-83x1077290.x.f32.dat","QCLOUDf48.bin.f32", "vx.f32"]
sz_error_bounds = ["sz2", "sz3", "sz4"]

def calculate(target_dir, target_data, sz_error_bound):
    cmd = "rm ./szdat && rm ./szhuff && rm ./szdat.lz4.huff && rm ./szdat.lz4 && rm ./szhuff.lz4"
    os.system(cmd)
    cmd1 = "cp ../{}/szdats/{}.{}.szdat ./szdat".format(target_dir, target_data, sz_error_bound)
    cmd2 = "cp ../{}/huffs/{}.{}.szhuff ./szhuff".format(target_dir, target_data, sz_error_bound)
    cmd3 = "lz4 -9 ./szdat ./szdat.lz4"
    cmd4 = "./huffman ./szdat.lz4 ./szdat.lz4.huff"
    cmd5 = "lz4 -9 ./szhuff ./szhuff.lz4"
    os.system(cmd1)
    os.system(cmd2)
    os.system(cmd3)
    os.system(cmd4)
    os.system(cmd5)
    ori_size = os.path.getsize("./szdat")
    huff_size = os.path.getsize("./szhuff")
    huff_lz4_size = os.path.getsize("./szhuff.lz4")
    lz4_size = os.path.getsize("./szdat.lz4")
    lz4_huff_size = os.path.getsize("./szdat.lz4.huff")
    cmd = 'echo "{},{},{},{},{},{},{},{}" >> result'.format(target_dir, target_data, sz_error_bound, ori_size, huff_size, huff_lz4_size, lz4_size, lz4_huff_size)
    os.system(cmd)
    return [ori_size, huff_size, huff_lz4_size, lz4_size, lz4_huff_size]

for i in range(4):
    for eb in sz_error_bounds:
        calculate(target_dirs[i], target_datas[i], eb)
