import os
import time
from enum import Enum
import shutil
import subprocess
import re

from multiprocessing import Pool
import tempfile
class Mode(Enum):
    MAC = ("../../src/EPAQ_FINAL/build/bin/sz", "mac")
    LPAQ = ("../../src/LPAQ_FINAL/build/bin/sz", "lpaq")
    ORISZ = ("../../src/ORISZ/build/bin/sz", "orisz")
    ADT_FSE = ("../../src/ADT_FSE/build/bin/sz", "adt_fse")
    ZFP = ("../../src/zfp/build/bin/zfp", "zfp")
    def __init__(self, path, name):
        self._path = path
        self._name = name

    @property
    def path(self):
        return self._path

    @property
    def name(self):
        return self._name


test_dict = {}
test_dict["CESM-ATM"] = ("CESM-ATM/SDRBENCH-CESM-ATM-26x1800x3600", 3, [26, 1800, 3600])
test_dict["EXAALT"] = ("EXAALT/SDRBENCH-exaalt-copper", 2, [83, 1077290])
test_dict["HuccianeISABEL"] = ("HurricaneISABEL/100x500x500", 3, [500, 500, 100])
test_dict["HACC"] = ("HACC/280953867", 1, [280953867])
modes = [Mode.ORISZ, Mode.MAC, Mode.LPAQ, Mode.ZFP, Mode.ADT_FSE] 
pool_size = 1 
data_dir = "../../datasets/"
for (dir_pos, paramnum, params) in test_dict.values():
    file_list = os.listdir(data_dir + dir_pos)[:15]
    for file in file_list:
        file = dir_pos + "/" + file
        for precision in ["1e-2", "1e-3", "1e-4"]:
            for mode in modes:    
                try:
                    flag_str = "{},{},{}".format(file, mode.name, precision)
                    test_flag = True
                    with open("./result") as f:
                        for line in f:
                            if flag_str in line:
                                test_flag = False
                    print("test_flag: {}, test file: {}, mode: {}, precision: {}". format(test_flag, file, mode, precision))
                except Exception as e:
                    print(e)
