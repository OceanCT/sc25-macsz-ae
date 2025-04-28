import os

sz_path = "../../src/SZ_PrintHuffmanSZData/build/bin/sz"
dir_path = "../../datasets/HACC/280953867/"
files = os.listdir(dir_path)[:10]
add_info = "-1 280953867"
szcs       = ["../szcs/szC2.config", 
             "../szcs/szC3.config",
             "../szcs/szC4.config",]
# sz -z -f -i dat.32 -c sz.config -3 26 1800 3600

for file in files:
    file_path = dir_path + file
    cp_cmd = "cp {} ./dat".format(file_path)
    os.system(cp_cmd)
    for j in range(4):
        # skip data already dealt 
        flag1 = False
        flag2 = False
        for i in os.listdir("./szdats"):
            if i == file+".sz.{}.szdat".format(str(j+2)):
                flag1 = True
        for i in os.listdir("./huffs"):
            if file in file+".sz{}.szhuff".format(str(j+2)):
                flag2 = True
        if flag1 and flag2:
            continue
        cmd1 = "{} -z -f -i dat -c {} {}".format(sz_path, szcs[j], add_info)
        os.system(cmd1)
        cmd2 = "mv ./szdat1 ./szdats/{}.sz{}.szdat".format(file, str(j+2))
        os.system(cmd2)
        cmd3 = "mv ./szdat.huff ./huffs/{}.sz{}.szhuff".format(file, str(j+2))
        os.system(cmd3)
