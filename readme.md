# Different Approaches

SZ -- original SZ compressor 

ADT-FSE-SZ -- Using addition adaptation table and fse to replace SZ's huffman encoder

MAC-SZ -- our approach, using tailored modeling-based arithmetic coding as the quantization factor encoder

LPAQ-SZ -- using lpaq as the quantization factor encoder

ZFP --  another well-known lossy compressor 

see these approaches in the "/src" directory

# Test Data:

The tested data we use in the paper are originally downloaded from the sdrbench page[https://sdrbench.github.io/].

Still, we provide the datasets we use in the "/data" directory. It includes four datasets: CESM-ATM, EXAALT, Hurricane ISABEL and EXAALT 

# Test Scripts 

Test scripts are divided into two parts: standalone test and hpc-cluster test. 
They are provided in the "/scripts" directory. For more information, see the readme in that directory. 

# Picture Draw 

Draw the picture in our paper with the scripts we provided in the "visualize" page.

