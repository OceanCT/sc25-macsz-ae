# Different Approaches

SZ -- original SZ compressor 

ADT-FSE-SZ -- Using addition adaptation table and fse to replace SZ's huffman encoder

MAC-SZ -- our approach, using tailored modeling-based arithmetic coding as the quantization factor encoder

LPAQ-SZ -- using lpaq as the quantization factor encoder

ZFP --  another well-known lossy compressor 

see these approaches and their implementations in the "/src" directory

# Test Data:

The tested data we use in the paper are originally downloaded from the sdrbench page[https://sdrbench.github.io/].

# How to get the pictures we draw in the paper

<!-- 0. to run all the tests and draw the picture in one command:
'''
'''

If the run-all script does not work, follow the following step to check which exact step went run and send issues to us if it cannot be easily solved. -->

1. compile all the approaches: 

'''
cd ./scripts && ./compile.sh 
'''

2. run the standalone tests, this might take about a day to finish:
'''
cd ./scripts/pw_rel_test/ 
nohup ./scripts/pw_rel_test/szt_monitor.sh > monitor_log 2>&1 &
'''

to check if the tests are all finished, use the scripts we provide here:
'''
cd ./scripts/pw_rel_test/ 
python3 progress.py 
'''
if all the test_flag are "False", it means finished, otherwise just wait a few more hours 

3. get the standalone evalution data we give in the paper:

'''
cd ./scripts/visualize && python3 preprocess.py
# Table 4 in stdout
cd ./scripts/visualize && python3 qfcr.py
# Table 5 in stdout
cd ./scripts/visualize && python3 cr.py
# Figure 9 in ./scripts/visualize/pics/CESM-ATM_0.01_qfsize.pdf
cd ./scripts/visualize && python3 qfr.py
# Figure 10 in ./scripts/visualize/pics/ctime.pdf && ./scripts/visualize/pics/dtime.pdf
cd ./scripts/visualize && python3 ct.py && python3 dt.py
# Figure 13 in ./scripts/prefix_test/prefix.pdf
# Notice that this process also takes a huge amount of time
cd ./scripts/prefix_test && python3 test.py
cd ./scripts/prefix_test && python3 draw.py
'''

4. the cluster tests 

Cluster tests are done in Tianhe-2 system, we simulate read and write using "pv" cmd and make sure the speed is the same as the given speed of SZ's original paper, namely 4.28TB/h dumping and 3Tb/h loading. 

run the following cmd to see Figure 11 and 12:
'''
cd ./scripts/visualize && python3 hpc.py
'''
See all the figures in ./scripts/hpc/*.pdf
