# 本周工作汇报

## 补全测试script(针对各个predictor测试有效性、补零个数、)
-》之前因为将size_t与int混用造成错误，虽然在本机可以正常解压缩，但是在ubuntu上出现错误

部分结果如下图:
![decompress_pic.svg](<Screenshot 2024-09-04 at 15.52.59.png>)
![alt text](<Screenshot 2024-09-04 at 16.46.15.png>)


-》bitpack补0个数比较关键，补7或16个0凑成满字节时压缩率几乎与直接使用lpaq无区别，否则差距较大

## 检查补零问题

2.1. Matchmodel的影响

matchmodel根据之前的字节预测当前字节的后几位，在之前都默认matchmodel对于整个预测有着较为重要的影响，符合bitpack位补零补成满字节影响较大的现象。但是本周回看之前尝试去除matchmodel以判断matchmodel是否对预测结果有影响的测试，发现可以调整部分，在调整过后，不经过matchmodel后的预测结果也相当准确，在大数据集上测试后也是相同结果，因此matchmodel部分可以移除。

2.2. 其他predictor

即使移除了matchmodel，测试显示在补7个0时压缩率（预测准确率）依旧最高。
对于不补零的情况，尝试在每一个新的数开始前清空之前的信息，压缩率结果确实比不清空要好许多但是距离补0至满字节差距较远。
同时这里仍然有补0至满字节比较不补零好许多的现象。
这里仍然需要进一步工作。


## 分析程序剩余的运行时长主要集中在哪一部分
-》主要时间都在执行predictor.update
![alt text](<Screenshot 2024-09-04 at 16.19.06.png>)
关闭所有预测器(1,2,3,4,6)，测试显示剩余时间主要在执行两个步骤：
(1) Mixer
```cpp
m.set(order+10*(h[0]>>13));
pr=m.p();
```
(2) APM
```cpp
  pr=pr+3*a1.pp(y, pr, c0)>>2;
  pr=pr+3*a2.pp(y, pr, c0^h[0]>>2)>>2;
```