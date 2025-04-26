#ifndef _SZ_BITPACKING
#define _SZ_BITPACKING

#include <stdlib.h>
#include <limits.h>
#include <assert.h>

const int order = 0;
const int addzero = 0;// 9 + 7 = 16 , 32

// assume sizeof(int) == 4, which means 32bits;
struct BitPacker {
    unsigned int buffer[32];
    int pointer;
};
unsigned int to_uint(struct BitPacker bp) {
    int res = 0;
    for(int i = 0; i < 32; i++) {
        res = res | (bp.buffer[i] << (31 - i));
    }
    return res;
}
int add_bit(struct BitPacker* bp, unsigned int s) {
    bp->buffer[bp->pointer++] = s;
    if(bp->pointer == 32) {
        bp->pointer = 0;
        return 1;
    } else {
        return 0;
    }
}

struct BitUnpacker {
    unsigned int buffer[64];
    int p1, p2;
    int bitwidth;
    int leagalsize;
};

void bitload(struct BitUnpacker* bp, unsigned int s) {
    for(int i = 0; i < 32; i++) {
        bp->buffer[bp->p2] = (s >> 31) & 1;
        s = s << 1;
        bp->p2 = (bp->p2 + 1) % 64;
    }
    bp->leagalsize += 32;
}
unsigned int bitextract(struct BitUnpacker* bp) {
    unsigned int res = 0;
    if(order == 0) {
        for(int i = 0; i < bp->bitwidth; i++) {
            res = res | (bp->buffer[bp->p1] << i);
            bp->p1 = (bp->p1 + 1) % 64;
        }
    } else {
        for(int i = 0; i < bp->bitwidth; i++) {
            res = res | (bp->buffer[bp->p1] << (bp->bitwidth - i - 1));
            bp->p1 = (bp->p1 + 1) % 64;
        }
    }
    bp->leagalsize -= bp->bitwidth;
    return res;
}

const int printflag = 0;
void print(struct BitUnpacker* bp) {
    if(!printflag) return;
    printf("bitwidth: %d, p1: %d, p2: %d\n", bp->bitwidth, bp->p1, bp->p2);
    for(int i = 0; i < 32; i++) {   
        printf("%d ", bp->buffer[i]);
    }
    printf("\n");
}


void bitpack(int *origin, size_t length, unsigned int** res, int* res_len, int* bitwidth, int* base) {
    assert(sizeof(int) == 4);
    *base = origin[0], *bitwidth = 0;
    int max = origin[0];
    for(int i = 1; i < length; i++) { 
        if(*base > origin[i]) {
            *base = origin[i];
        }
        if(origin[i] > max) {
            max = origin[i];
        }
    }
    unsigned int tmp = max - (*base);    
    while(tmp) {
        (*bitwidth)++;
        tmp >>= 1;
    }
    *bitwidth = *bitwidth + addzero;
    *res_len = ((long long)length * (*bitwidth) + 31) / 32;

    // if bitwidth is 0, it means all numbers are the same
    if(*bitwidth == 0) {
        printf("bitwidth: %d, all numbers are the same, so return\n", *bitwidth);
        return;
    }
    *res = (unsigned int*)malloc((*res_len) * sizeof(unsigned int));
    int pos = 0;
    struct BitPacker bp;
    bp.pointer = 0;
    for(int i = 0; i < length; i++) {
        unsigned int tmp = (origin[i] - *base);
        if(order == 0) {
            for(int j = 0; j < *bitwidth; j++) {
                int add = add_bit(&bp, tmp & 1);
                tmp >>= 1;
                if(add) {
                    (*res)[pos++] = to_uint(bp);
                }
            }
        } else {
            for(int j = 0; j < *bitwidth; j++) {
                int add = add_bit(&bp, (tmp >> (*bitwidth - 1 - j)) & 1);
                if(add) {
                    (*res)[pos++] = to_uint(bp);
                }
            }
        }
    }
    if(pos < *res_len) { 
        (*res)[pos] = to_uint(bp);
    }
}

// res should already be allocated
void bitunpack(unsigned int* origin, int length, int bitwidth, int base, int* res, size_t res_len) {   
    struct BitUnpacker bp;
    bp.leagalsize = 0, bp.p1 = 0, bp.p2 = 0;
    bp.bitwidth = bitwidth;
    bp.p1 = 0, bp.p2 = 0;
    int p1 = 0, p2 = 0;
    while(p1 < length) {
        bitload(&bp, origin[p1++]);
        while(bp.leagalsize >= bitwidth && p2 < res_len) {
            unsigned int tmp = bitextract(&bp);
            res[p2++] = tmp + base;
        }
    }
}

#endif