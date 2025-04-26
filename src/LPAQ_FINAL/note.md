# LPAQ的压缩与解压原理

首先看到LPAQ的压缩

```cpp
// code(y)
int p = predictor.p();
p += p < 2048;
U32 xmid=x1 + ((x2-x1)>>12)*p + ((x2-x1&0xfff)*p>>12);
y ? (x2=xmid) : (x1=xmid+1);
predictor.update(y);
while (((x1^x2)&0xff000000)==0) {  
    // printf("code output\n");
    if(tmpsize<=totalCnt) {
    U8* buffer;
    alloc(buffer, tmpsize*2);
    memcpy(buffer, *outputbits, totalCnt);
    free(*outputbits);
    *outputbits = buffer;
    tmpsize *= 2;
    }
    (*outputbits)[totalCnt++] = x2>>24;
    x1<<=8;
    x2=(x2<<8)+255;
}
```