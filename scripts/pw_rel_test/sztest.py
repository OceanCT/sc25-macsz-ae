import os
import time
from enum import Enum
import shutil
import subprocess
import re

from multiprocessing import Pool
import tempfile
class ConfigFile:
    def __init__(self):
        self.sections = {}

    def add_section(self, section_name):
        """Add a new section to the config file."""
        if section_name not in self.sections:
            self.sections[section_name] = {}

    def set(self, section_name, key, value):
        """Set a key-value pair under a specific section."""
        if section_name not in self.sections:
            raise ValueError("Section {} does not exist.".format(section_name))
        self.sections[section_name][key] = value

    def write_to_file(self, filename):
        """Write the configuration to a file."""
        with open(filename, 'w') as f:
            for section, params in self.sections.items():
                f.write("[{}]\n".format(section))
                for key, value in params.items():
                    f.write("{} = {}\n".format(key, value))
                f.write("\n")

config = ConfigFile()

# Add sections
config.add_section("ENV")
config.add_section("PARAMETER")
config.set("ENV", "dataEndianType", "LITTLE_ENDIAN_DATA")
config.set("ENV", "sol_name", "SZ")
config.set("PARAMETER", "snapshotCmprStep", 5)
config.set("PARAMETER", "withLinearRegression", "YES")
config.set("PARAMETER", "protectValueRange", "NO")
config.set("PARAMETER", "sampleDistance", 100)
config.set("PARAMETER", "quanzitation_intervals", 0)
config.set("PARAMETER", "max_quant_intervals", 65535)
config.set("PARAMETER", "predThreshold", 0.99)
config.set("PARAMETER", "szMode", "SZ_BEST_COMPRESSION")
config.set("PARAMETER", "losslessCompressor", "ZSTD_COMPRESSOR")
config.set("PARAMETER", "gzipMode", "Gzip_BEST_COMPRESSION")
config.set("PARAMETER", "zstdMode", "Zstd_BEST_COMPRESSION")
config.set("PARAMETER", "randomAccess", 0)
config.set("PARAMETER", "errorBoundMode", "PW_REL")
config.set("PARAMETER", "absErrBound", 4e-7)
config.set("PARAMETER", "relBoundRatio", "1E-2")
config.set("PARAMETER", "psnr", 80)
config.set("PARAMETER", "normErr", 0.05)
config.set("PARAMETER", "pw_relBoundRatio", "1E-2")
config.set("PARAMETER", "accelerate_pw_rel_compression", 0)

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

def exe_command(command):
    tmp_file = "./tmp"
    new_command = command + " > " + tmp_file + " 2>&1"
    print(new_command)
    os.system(new_command)
    with open(tmp_file, 'r') as f:
        content = f.read()
    return content

data_dir = "../../datasets/"
def test_file(args):
    filename, mode, paramnum, params, precision = args
    
    with tempfile.TemporaryDirectory() as temp_dir:
        base_filename = os.path.basename(filename)  
        temp_filename = os.path.join(temp_dir, base_filename)
        shutil.copy(os.path.join(data_dir, filename), temp_filename)
        ori_size = os.stat(temp_filename).st_size
        
        param_str = " ".join([str(x) for x in params])
        qf_size = "/"
        qf_size_compressed = "/"

        if mode == Mode.ZFP:
            p_convert = 16
            if precision == "1e-2":
                p_convert = 16
            if precision == "1e-3":
                p_convert = 19
            if precision == "1e-4":
                p_convert = 23
            if precision == "1e-5":
                p_convert = 26
            cmd1 = f"{mode.path} -f -i {temp_filename} -z {temp_filename}.zfp -{paramnum} {param_str} -c 0 0 {p_convert} -1075"
            cmd2 = f"{mode.path} -f -z {temp_filename}.zfp -o {temp_filename}.zfp.out -{paramnum} {param_str} -c 0 0 {p_convert} -1075"
            
            t_0 = time.time()
            result1 = os.system(cmd1)
            t_1 = time.time()
            result2 = os.system(cmd2)
            t_2 = time.time()

            ctime = t_1 - t_0
            dtime = t_2 - t_1
            compressed_size = os.stat(f"{temp_dir}/{base_filename}.zfp").st_size
            cratio = ori_size / compressed_size
        else:
            """
            if mode != Mode.ADT_FSE:
                config.set("PARAMETER", "max_quant_intervals", 1048575)
            else:
                config.set("PARAMETER", "max_quant_intervals", 65535)
            """
            config.set("PARAMETER", "pw_relBoundRatio", precision)
            config.write_to_file(os.path.join(temp_dir, "sz.config"))
            
            command1 = f"{mode.path} -z -f -i {temp_filename} -c {temp_dir}/sz.config -{paramnum} {param_str}"
            command2 = f"{mode.path} -x -f -i {temp_filename} -c {temp_dir}/sz.config -{paramnum} {param_str} -s {temp_filename}.sz -a"
            
            print(command1)
            result = os.popen(command1).read()
            print("res:", result)
            
            ctime_re = r'compression time = (.*)\n'
            qf_size_compressed_re0 = r'typeArray compressed size: (.*)\n'
            qf_size_re = r'Quantization Factor Size: {(.*)}\n'
            qf_size_compressed_re = r'Quantization Factor Compressed Size: {(.*)}\n'
            
            ctime = re.findall(ctime_re, result)[0]
            try:
                if re.findall(qf_size_compressed_re0, result):
                    qf_size_compressed = re.findall(qf_size_compressed_re0, result)[0]
                else:
                    qf_size = re.findall(qf_size_re, result)[0]
                    qf_size_compressed = re.findall(qf_size_compressed_re, result)[0]
            except Exception:
                pass
            
            print(command2)
            result = os.popen(command2).read()
            print(result)
            
            dtime_re = r'decompression time = (.*) seconds'
            cratio_re = r'compressionRatio=(.*)\n'
            
            dtime = re.findall(dtime_re, result)[0]
            cratio = re.findall(cratio_re, result)[0]
            compressed_size = os.stat(f"{temp_dir}/{base_filename}.sz").st_size
        
        result_str = f"{filename},{mode.name},{precision},{ori_size},{compressed_size},{cratio},{qf_size},{qf_size_compressed},{ctime},{dtime}"
        print(result_str)
        os.system(f'echo "{result_str}" >> result')


test_dict = {}
test_dict["CESM-ATM"] = ("CESM-ATM/SDRBENCH-CESM-ATM-26x1800x3600", 3, [26, 1800, 3600])
test_dict["EXAALT"] = ("EXAALT/SDRBENCH-exaalt-copper", 2, [83, 1077290])
test_dict["HuccianeISABEL"] = ("HurricaneISABEL/100x500x500", 3, [500, 500, 100])
test_dict["HACC"] = ("HACC/280953867", 1, [280953867])
modes = [Mode.ORISZ, Mode.MAC, Mode.LPAQ, Mode.ZFP, Mode.ADT_FSE] 
pool_size = 1 
tasks = []
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
                    if test_flag:
                        tasks.append((file, mode, paramnum, params, precision))
                except Exception as e:
                    print(e)
                    print("fail to test file:", file)

with Pool(pool_size) as pool:
    pool.map(test_file, tasks)
