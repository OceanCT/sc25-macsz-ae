#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "fse.h"
#include "fsed.h"

typedef unsigned char U8;
typedef struct {
    int code;
    int bitCount;
    uint64_t extraBits;
} ADTCode;

ADTCode adt_map(int value, int intvRadius) {
    ADTCode result;
    result.extraBits = 0; // Initialize extraBits
    if(value == -intvRadius) {
        result.code = 67;
        result.bitCount = 0;
    } else if (value >= 0 && value <= 15) {
        result.code = value;
        result.bitCount = 0;
    } else if (value >= 16 && value <= 23) {
        result.code = 16 + (value - 16) / 2;
        result.bitCount = 1;
        result.extraBits = (value - 16) % 2;
    } else if (value >= 24 && value <= 31) {
        result.code = 20 + (value - 24) / 4;
        result.bitCount = 2;
        result.extraBits = (value - 24) % 4;
    } else if (value >= 32 && value <= 47) {
        result.code = 22 + (value - 32) / 8;
        result.bitCount = 3;
        result.extraBits = (value - 32) % 8;
    } else if (value >= 48 && value <=63) {
        result.code = 24;
        result.bitCount = 4;
        result.extraBits = value - 48;
    } else if (value >= 64 && value <= 32767) {
        int powerOfTwo = 64;
        result.code = 25;
        result.bitCount = 0;
        while (powerOfTwo * 2 <= value) {
            powerOfTwo *= 2;
            result.code++;
        }
        result.bitCount = result.code - 19;
        result.extraBits = value - powerOfTwo;
    } else if (value >= -32767 && value <= -64) {
        int powerOfTwo = 64;
        result.code = 42;
        result.bitCount = 0;
        while (powerOfTwo * 2 <= -value) {
            powerOfTwo *= 2;
            result.code--;
        }
        result.bitCount = 48 - result.code;
        result.extraBits = (-value) - powerOfTwo;
    } else if(value >= -63 && value <= -48) {
        result.code = 43;
        result.bitCount = 4;
        result.extraBits = (-value - 48) ;
    } else if (value >= -47 && value <= -32) {
        result.code = 44 + (-value - 32) / 8;
        result.bitCount = 3;
        result.extraBits = (-value - 32) % 8;
    } else if (value >= -31 && value <= -24) {
        result.code = 46 + (-value - 24) / 4;
        result.bitCount = 2;
        result.extraBits = (-value - 24) % 4;
    } else if (value >= -23 && value <= -16) {
        result.code = 48 + (-value - 16) / 2;
        result.bitCount = 1;
        result.extraBits = (-value - 16) % 2;
    } else if (value >= -15 && value <= -1) {
        result.code = 52 + (-value - 1);
        result.bitCount = 0;
    } else if (value == -intvRadius) {
        result.code = 67;
        result.bitCount = 0;
    } else {
        printf("value: %d\n", value);
        result.code = -1; // Indicate an error
        result.bitCount = -1;
    }
    return result;
}
int inverse_adt_map(ADTCode adt, int intvRadius) {
    int value;
    if (adt.code >= 0 && adt.code <= 15 && adt.bitCount == 0) {
        value = adt.code;
    } else if (adt.code >= 16 && adt.code <= 19 && adt.bitCount == 1) {
        value = 16 + (adt.code - 16) * 2 + adt.extraBits;
    } else if (adt.code >= 20 && adt.code <= 21 && adt.bitCount == 2) {
        value = 24 + (adt.code - 20) * 4 + adt.extraBits;
    } else if (adt.code >= 22 && adt.code <= 23 && adt.bitCount == 3) {
        value = 32 + (adt.code - 22) * 8 + adt.extraBits;
    } else if (adt.code == 24 && adt.bitCount == 4) {
        value = 48+ adt.extraBits;
    } else if (adt.code >= 25 && adt.code <= 33 && adt.bitCount == (adt.code - 19)) {
        value = (1 << (adt.bitCount)) + adt.extraBits;
    } else if (adt.code >= 34 && adt.code <= 42 && adt.bitCount == (48 - adt.code)) {
        value = -( (1 << adt.bitCount) + adt.extraBits );
    } else if (adt.code == 43 && adt.bitCount == 4) {
        value = -(48+ adt.extraBits);
    } else if (adt.code >= 44 && adt.code <= 45 && adt.bitCount == 3) {
        value = -32 - (adt.code - 44) * 8 - adt.extraBits;
    } else if (adt.code >= 46 && adt.code <= 47 && adt.bitCount == 2) {
        value = -24 - (adt.code - 46) * 4 - adt.extraBits;
    } else if (adt.code >= 48 && adt.code <= 51 && adt.bitCount == 1) {
        value = -16 - (adt.code - 48) * 2 - adt.extraBits;
    } else if (adt.code >= 52 && adt.code <= 66 && adt.bitCount == 0) {
        value = -1 - (adt.code - 52);
    } else if (adt.code == 67 && adt.bitCount == 0) {
        value = -intvRadius;
    } else {
        printf("code: %d, bitCount:%d\n", adt.code, adt.bitCount);
        // Handle invalid ADTCode, perhaps return a special value or raise an error
       return -32769; // Indicate an error
    }
    return value;
}

int getbitcount(int code) {

    int bitCount = 0;

    // 根据 code 确定 bitCount
    if (code >= 16 && code <= 19) bitCount = 1;
    else if (code >= 20 && code <= 21) bitCount = 2;
    else if (code >= 22 && code <= 23) bitCount = 3;
    else if (code == 24) bitCount = 4;
    else if (code >= 25 && code <= 33) bitCount = code - 19;
    else if (code >= 34 && code <= 42) bitCount = 48 - code;
    else if (code == 43) bitCount = 4;
    else if (code >= 44 && code <= 45) bitCount = 3;
    else if (code >= 44 && code <= 45) bitCount = 3;
    else if (code >= 46 && code <= 47) bitCount = 2;
    else if (code >= 48 && code <= 51) bitCount = 1;
    return bitCount;
}


uint32_t next_power_of_two(uint32_t n) {
    if (n == 0) return 1;

    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n + 1;
}

void adt_fse_compress(int* content, size_t length, unsigned char** output, size_t* output_size) {
    // *output_size = length * 4;
    // *output = (unsigned char*)malloc(*output_size * sizeof(unsigned char));
    // memcpy(*output, content, *output_size);
    // return;
    // 1. 计算 intvRadius
    // length = 20;
    ADTCode* adt_array = (ADTCode*)malloc(length * sizeof(ADTCode));
    unsigned char* codes = (unsigned char*)malloc(length);

    int max_val = 0;
    for (size_t i = 0; i < length; ++i) {
        if (abs(content[i]) > max_val) max_val = abs(content[i]);
    }
    int intvRadius = next_power_of_two(max_val) / 2;

    // 先跑 adt_map 拿到 code + extra bits
    for (size_t i = 0; i < length; ++i) {
        adt_array[i] = adt_map(content[i] - intvRadius, intvRadius);
        codes[i] = (unsigned char)adt_array[i].code;
    }
    size_t totalBitCount = 0;
    for (size_t i = 0; i < length; ++i) {
        totalBitCount += adt_array[i].bitCount;
    }
    size_t extraBitsSize = (totalBitCount + 7) / 8;
    unsigned char* extraBitsBuf = (unsigned char*)calloc(extraBitsSize, 1);

    // 写入每个 bit
    size_t bit_index = 0;
    for (size_t i = 0; i < length; ++i) {
        uint32_t extraBits = adt_array[i].extraBits;
        int bitCount = adt_array[i].bitCount;

        for (int b = 0; b < bitCount; ++b) {
            int bit = (extraBits >> b) & 1;
            if (bit) {
                extraBitsBuf[bit_index / 8] |= (1 << (bit_index % 8));
            }
            ++bit_index;
        }
    }


    // 3. FSE 编码
    size_t maxDstSize = FSE_compressBound(length);
    unsigned char* fseCompressed = (unsigned char*)malloc(maxDstSize);
    // /*
    // size_t fseSize = length * 4;
    // fseCompressed = codes;
    // */
    size_t fseSize = FSE_compress(fseCompressed, maxDstSize, codes, length);
    assert(!FSE_isError(fseSize));
    // int total_bit_count = 0;
    // for (int i = 0; i < length; ++i) {
    //     total_bit_count += getbitcount(codes[i]);
    // }
    // 4. 数据布局：[length (4)][intvRadius (4)][fseSize (4)][fseData][extraSize (4)][extraBits]
    *output_size = 4 + 4 + 4 + fseSize + 4 + extraBitsSize;
    printf("fse_size: %ld, extraBitsSize: %d, intvRadius:%d\n", fseSize, extraBitsSize, intvRadius);
    // for(int i = length-1; i >= length- 20; i--) {
    //     printf("%d ", content[i]);
    // }
    // printf("\n");
    *output = (unsigned char*)malloc(*output_size);
    
    unsigned char* p = *output;

    // 存储原始数据长度
    uint32_t val32;
    val32 = (uint32_t)length;
    memcpy(p, &val32, 4); p += 4;

    // 填充 intvRadius
    val32 = (uint32_t)intvRadius;
    memcpy(p, &val32, 4); p += 4;

    // 填充 fseSize
    val32 = (uint32_t)fseSize;
    memcpy(p, &val32, 4); p += 4;

    // 填充 FSE 压缩数据
    memcpy(p, fseCompressed, fseSize); p += fseSize;

    // 填充 extraBitsSize
    val32 = (uint32_t)extraBitsSize;
    memcpy(p, &val32, 4); p += 4;

    // 填充 extraBitsBuf
    memcpy(p, extraBitsBuf, extraBitsSize);

    // // 释放内存
    free(codes);
    free(fseCompressed);
    free(extraBitsBuf);
    // test decompression 
	// int* new1 = (int*)malloc(length * sizeof(int));
	// adt_fse_decompress(*output, new1, length);
	// for(int i = 0; i < length; i++) {
	// 	if(new1[i] != content[i]) {
	// 		printf("new1[%d]: %d, content[%d]: %d\n", i, new1[i], i, content[i]);
	// 	}
	// 	assert(new1[i] == content[i]);
	// }
}

void adt_fse_decompress(unsigned char* content, int* out, int count) {
    unsigned char* p = content;

    uint32_t orig_length, intvRadius, fse_size, extra_size;
    
    // 读取原始数据的长度
    memcpy(&orig_length, p, 4); p += 4;

    // 如果传入的 count 与存储的 length 不符，可以在这里进行校验
    assert(orig_length == count);  // 确保解压时长度与原始长度一致

    // 读取 intvRadius 和 fse_size
    memcpy(&intvRadius, p, 4); p += 4;
    memcpy(&fse_size, p, 4); p += 4;

    unsigned char* fse_data = p; p += fse_size;

    // 读取 extra_size 和 extra_data
    memcpy(&extra_size, p, 4); p += 4;
    unsigned char* extra_data = p;

    // 计算 total_bit_count（所有 code 对应的 bitCount 的总和）
    unsigned char* code_buf = (unsigned char*)malloc(count);
    size_t result = FSE_decompress(code_buf, count, fse_data, fse_size);
    assert(result == count);  // 确保 FSE 解压成功
    printf("extra_size: %d, fseSize: %d\n", extra_size, fse_size);


    // 遍历所有 code 计算总的 bitCount
    // for (int i = 0; i < count; ++i) {
    //     total_bit_count += getbitcount(code_buf[i]);
    // }

    // 解压 extraBits 数据
    size_t bit_index = 0;
    for (int i = 0; i < count; ++i) {
        int code = code_buf[i];
        int bitCount = getbitcount(code);

        // 读取 extraBits
        int64_t extraBits = 0;
        for (int b = 0; b < bitCount; ++b) {
            extraBits |= ((extra_data[bit_index / 8] >> (bit_index % 8)) & 1) << b;
            ++bit_index;
        }
        
        // 恢复原始数据
        ADTCode tmpc;
        tmpc.code = code;
        tmpc.extraBits = extraBits;
        tmpc.bitCount = bitCount;
        out[i] = intvRadius + inverse_adt_map(tmpc, intvRadius);
    }
    free(code_buf);
}
