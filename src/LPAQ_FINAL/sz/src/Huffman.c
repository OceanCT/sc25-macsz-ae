/**
 *  @file Huffman.c
 *  @author Sheng Di
 *  @date Aug., 2016
 *  @brief Customized Huffman Encod9ing, Compression and Decompression functions
 *  (C) 2016 by Mathematics and Computer Science (MCS), Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys/time.h"
#include "Huffman.h"
#include "sz.h"
#include "lpaq.h"
#include "bitpacking.h"
#include <stdlib.h>
#define USE_LPAQ 1
#define LPAQ_LEVEL 3
#define USE_BITPACK 0

HuffmanTree* createHuffmanTree(int stateNum)
{
	// printf("CreateHuffmanTree, stateNum: %d\n", stateNum);
	HuffmanTree *huffmanTree = (HuffmanTree*)malloc(sizeof(HuffmanTree));
	memset(huffmanTree, 0, sizeof(HuffmanTree));
	huffmanTree->stateNum = stateNum;
	huffmanTree->allNodes = 2*stateNum;

	huffmanTree->pool = (struct node_t*)malloc(huffmanTree->allNodes*2*sizeof(struct node_t));
	huffmanTree->qqq = (node*)malloc(huffmanTree->allNodes*2*sizeof(node));
	huffmanTree->code = (uint64_t**)malloc(huffmanTree->stateNum*sizeof(uint64_t*));
	huffmanTree->cout = (unsigned char *)malloc(huffmanTree->stateNum*sizeof(unsigned char));

	memset(huffmanTree->pool, 0, huffmanTree->allNodes*2*sizeof(struct node_t));
	memset(huffmanTree->qqq, 0, huffmanTree->allNodes*2*sizeof(node));
    memset(huffmanTree->code, 0, huffmanTree->stateNum*sizeof(uint64_t*));
    memset(huffmanTree->cout, 0, huffmanTree->stateNum*sizeof(unsigned char));
	huffmanTree->qq = huffmanTree->qqq - 1;
	huffmanTree->n_nodes = 0;
    huffmanTree->n_inode = 0;
    huffmanTree->qend = 1;

    return huffmanTree;
}

HuffmanTree* createDefaultHuffmanTree()
{
	int maxRangeRadius = 32768;
	int stateNum = maxRangeRadius << 1; //*2

    return createHuffmanTree(stateNum);
}

node new_node(HuffmanTree* huffmanTree, size_t freq, unsigned int c, node a, node b)
{
	node n = huffmanTree->pool + huffmanTree->n_nodes++;
	if (freq)
	{
		n->c = c;
		n->freq = freq;
		n->t = 1;
	}
	else {
		n->left = a;
		n->right = b;
		n->freq = a->freq + b->freq;
		n->t = 0;
		//n->c = 0;
	}
	return n;
}

node new_node2(HuffmanTree *huffmanTree, unsigned int c, unsigned char t)
{
	huffmanTree->pool[huffmanTree->n_nodes].c = c;
	huffmanTree->pool[huffmanTree->n_nodes].t = t;
	return huffmanTree->pool + huffmanTree->n_nodes++;
}

/* priority queue */
void qinsert(HuffmanTree *huffmanTree, node n)
{
	int j, i = huffmanTree->qend++;
	while ((j = (i>>1)))  //j=i/2
	{
		if (huffmanTree->qq[j]->freq <= n->freq) break;
		huffmanTree->qq[i] = huffmanTree->qq[j], i = j;
	}
	huffmanTree->qq[i] = n;
}

node qremove(HuffmanTree* huffmanTree)
{
	int i, l;
	node n = huffmanTree->qq[i = 1];
	node p;
	if (huffmanTree->qend < 2) return 0;
	huffmanTree->qend --;
	huffmanTree->qq[i] = huffmanTree->qq[huffmanTree->qend];

	while ((l = (i<<1)) < huffmanTree->qend)  //l=(i*2)
	{
		if (l + 1 < huffmanTree->qend && huffmanTree->qq[l + 1]->freq < huffmanTree->qq[l]->freq) l++;
		if(huffmanTree->qq[i]->freq > huffmanTree->qq[l]->freq)
		{
			p = huffmanTree->qq[i];
			huffmanTree->qq[i] = huffmanTree->qq[l];
			huffmanTree->qq[l] = p;
			i = l;
		}
		else
		{
			break;
		}

	}

	return n;
}

/* walk the tree and put 0s and 1s */
/**
 * @out1 should be set to 0.
 * @out2 should be 0 as well.
 * @index: the index of the byte
 * */
void build_code(HuffmanTree *huffmanTree, node n, int len, uint64_t out1, uint64_t out2)
{
	if (n->t) {
		huffmanTree->code[n->c] = (uint64_t*)malloc(2*sizeof(uint64_t));
		if(len<=64)
		{
			(huffmanTree->code[n->c])[0] = out1 << (64 - len);
			(huffmanTree->code[n->c])[1] = out2;
		}
		else
		{
			(huffmanTree->code[n->c])[0] = out1;
			(huffmanTree->code[n->c])[1] = out2 << (128 - len);
		}
		huffmanTree->cout[n->c] = (unsigned char)len;
		return;
	}
	int index = len >> 6; //=len/64
	if(index == 0)
	{
		out1 = out1 << 1;
		out1 = out1 | 0;
		build_code(huffmanTree, n->left, len + 1, out1, 0);
		out1 = out1 | 1;
		build_code(huffmanTree, n->right, len + 1, out1, 0);
	}
	else
	{
		if(len%64!=0)
			out2 = out2 << 1;
		out2 = out2 | 0;
		build_code(huffmanTree, n->left, len + 1, out1, out2);
		out2 = out2 | 1;
		build_code(huffmanTree, n->right, len + 1, out1, out2);
	}
}

/**
 * Compute the frequency of the data and build the Huffman tree
 * @param HuffmanTree* huffmanTree (output)
 * @param int *s (input)
 * @param size_t length (input)
 * */
void init(HuffmanTree* huffmanTree, int *s, size_t length)
{
	// printf("Init HuffmanTree, length:%d\n", length);
	size_t tmp_i;
	// for(tmp_i = 0; tmp_i < length; tmp_i++) {
	// 	printf("%d ", s[tmp_i]);
	// }
	// printf("\n");
	size_t i, index;
	size_t *freq = (size_t *)malloc(huffmanTree->allNodes*sizeof(size_t));
	memset(freq, 0, huffmanTree->allNodes*sizeof(size_t));
	for(i = 0;i < length;i++)
	{
		index = s[i];
		freq[index]++;
	}

	for (i = 0; i < huffmanTree->allNodes; i++)
		if (freq[i])
			qinsert(huffmanTree, new_node(huffmanTree, freq[i], i, 0, 0));

	while (huffmanTree->qend > 2)
		qinsert(huffmanTree, new_node(huffmanTree, 0, 0, qremove(huffmanTree), qremove(huffmanTree)));

	build_code(huffmanTree, huffmanTree->qq[1], 0, 0, 0);
	free(freq);
}

void init_static(HuffmanTree* huffmanTree, int *s, size_t length)
{
	size_t i;
	size_t *freq = (size_t *)malloc(huffmanTree->allNodes*sizeof(size_t));
	memset(freq, 0, huffmanTree->allNodes*sizeof(size_t));


	for (i = 0; i < huffmanTree->allNodes; i++)
		if (freq[i])
			qinsert(huffmanTree, new_node(huffmanTree, freq[i], i, 0, 0));

	while (huffmanTree->qend > 2)
		qinsert(huffmanTree, new_node(huffmanTree, 0, 0, qremove(huffmanTree), qremove(huffmanTree)));

	build_code(huffmanTree, huffmanTree->qq[1], 0, 0, 0);
	free(freq);
}

void encode(HuffmanTree *huffmanTree, int *s, size_t length, unsigned char *out, size_t *outSize)
{	
	size_t i = 0;
	unsigned char bitSize = 0, byteSize, byteSizep;
	int state;
	unsigned char *p = out;
	int lackBits = 0;
	//int64_t totalBitSize = 0, maxBitSize = 0, bitSize21 = 0, bitSize32 = 0;
	for (i = 0;i<length;i++)
	{
		state = s[i];
		bitSize = huffmanTree->cout[state];

		if(lackBits==0)
		{
			byteSize = bitSize%8==0 ? bitSize/8 : bitSize/8+1; //it's equal to the number of bytes involved (for *outSize)
			byteSizep = bitSize/8; //it's used to move the pointer p for next data
			if(byteSize<=8)
			{
				longToBytes_bigEndian(p, (huffmanTree->code[state])[0]);
				p += byteSizep;
			}
			else //byteSize>8
			{
				longToBytes_bigEndian(p, (huffmanTree->code[state])[0]);
				p += 8;
				longToBytes_bigEndian(p, (huffmanTree->code[state])[1]);
				p += (byteSizep - 8);
			}
			*outSize += byteSize;
			lackBits = bitSize%8==0 ? 0 : 8 - bitSize%8;
		}
		else
		{
			*p = (*p) | (unsigned char)((huffmanTree->code[state])[0] >> (64 - lackBits));
			if(lackBits < bitSize)
			{
				p++;
				//(*outSize)++;
				int64_t newCode = (huffmanTree->code[state])[0] << lackBits;
				longToBytes_bigEndian(p, newCode);

				if(bitSize<=64)
				{
					bitSize -= lackBits;
					byteSize = bitSize%8==0 ? bitSize/8 : bitSize/8+1;
					byteSizep = bitSize/8;
					p += byteSizep;
					(*outSize)+=byteSize;
					lackBits = bitSize%8==0 ? 0 : 8 - bitSize%8;
				}
				else //bitSize > 64
				{
					byteSizep = 7; //must be 7 bytes, because lackBits!=0
					p+=byteSizep;
					(*outSize)+=byteSize;

					bitSize -= 64;
					if(lackBits < bitSize)
					{
						*p = (*p) | (unsigned char)((huffmanTree->code[state])[0] >> (64 - lackBits));
						p++;
						//(*outSize)++;
						newCode = (huffmanTree->code[state])[1] << lackBits;
						longToBytes_bigEndian(p, newCode);
						bitSize -= lackBits;
						byteSize = bitSize%8==0 ? bitSize/8 : bitSize/8+1;
						byteSizep = bitSize/8;
						p += byteSizep;
						(*outSize)+=byteSize;
						lackBits = bitSize%8==0 ? 0 : 8 - bitSize%8;
					}
					else //lackBits >= bitSize
					{
						*p = (*p) | (unsigned char)((huffmanTree->code[state])[0] >> (64 - bitSize));
						lackBits -= bitSize;
					}
				}
			}
			else //lackBits >= bitSize
			{
				lackBits -= bitSize;
				if(lackBits==0)
					p++;
			}
		}
	}
//	for(i=0;i<stateNum;i++)
//		if(code[i]!=NULL) free(code[i]);
	/*printf("max bitsize = %d\n", maxBitSize);
	printf("bitSize21 ratio = %f\n", ((float)bitSize21)/length);
	printf("bitSize32 ratio = %f\n", ((float)bitSize32)/length);
	printf("avg bit size = %f\n", ((float)totalBitSize)/length);*/
}

void decode(unsigned char *s, size_t targetLength, node t, int *out)
{
	size_t i = 0, byteIndex = 0, count = 0;
	int r;
	node n = t;

	if(n->t) //root->t==1 means that all state values are the same (constant)
	{
		for(count=0;count<targetLength;count++)
			out[count] = n->c;
		return;
	}

	for(i=0;count<targetLength;i++)
	{

		byteIndex = i>>3; //i/8
		r = i%8;
		if(((s[byteIndex] >> (7-r)) & 0x01) == 0)
			n = n->left;
		else
			n = n->right;

		if (n->t) {
			//putchar(n->c);
			out[count] = n->c;
			n = t;
			count++;
		}
	}
//	putchar('\n');
	if (t != n) printf("garbage input\n");
	return;
}

void decode_MSST19(unsigned char *s, size_t targetLength, node t, int *out, int maxBits)
{
	size_t count = 0;
	node n = t;

	if(n->t) //root->t==1 means that all state values are the same (constant)
	{
		for(count=0;count<targetLength;count++)
			out[count] = n->c;
		return;
	}

	if(maxBits > 16){
		maxBits = 16;
	}

    int tableSize = 1 << maxBits;
    int* valueTable = (int*)malloc(tableSize * sizeof(int));
    uint8_t* lengthTable = (uint8_t*)malloc(tableSize * sizeof(int));
    node* nodeTable = (node*)malloc(tableSize * sizeof(node));
    uint32_t* maskTable = (uint32_t*) malloc((maxBits+8) * sizeof(uint32_t));
    int j;
    for(uint32_t i=0; i<tableSize; i++){
        n = t;
        j = 0;
        while(!n->t && j < maxBits){
            uint32_t res = i >> (maxBits - j - 1);
            if((res & 0x00000001) == 0){
                n = n->left;
            }else{
                n = n->right;
            }
            j++;
        }
        if(!n->t){
        	nodeTable[i] = n;
        	valueTable[i] = -1;
        	lengthTable[i] = maxBits;
        }else{
			valueTable[i] = n->c;
			lengthTable[i] = j;
        }
    }
    for(int i=0; i<maxBits+8; i++){
        maskTable[i] = (1 << (maxBits+8-i-1)) - 1;
    }

    int leftBits = 0;
	uint32_t currentValue = 0;
	size_t i = 0;

    while(count<targetLength)
	{
	    while(leftBits < maxBits){
	        currentValue = currentValue << 8;
	        currentValue += s[i];
	        leftBits += 8;
	        i++;
	    }

        uint32_t index = currentValue >> (leftBits - maxBits);
        int value = valueTable[index];
        if(value != -1){
			out[count] = value;
			int bitLength = lengthTable[index];
			leftBits -= bitLength;
			uint32_t avoidHeadMask = maskTable[maxBits + 8 - leftBits - 1];
			currentValue = (currentValue & avoidHeadMask);
			count++;
        }else{
			int bitLength = lengthTable[index];
			leftBits -= bitLength;
        	n = nodeTable[index];
        	while(!n->t){
        		if(!leftBits){
					currentValue = currentValue << 8;
					currentValue += s[i];
					leftBits += 8;
					i++;
        		}
				if(((currentValue >> (leftBits - 1)) & 0x01) == 0)
					n = n->left;
				else
					n = n->right;
				leftBits--;
        	}
        	currentValue &= maskTable[maxBits + 8 - leftBits - 1];
			out[count] = n->c;
			count++;
        }

	}
    free(valueTable);
    free(lengthTable);
    free(nodeTable);
    free(maskTable);
	return;
}
void pad_tree_uchar(HuffmanTree* huffmanTree, unsigned char* L, unsigned char* R, unsigned int* C, unsigned char* t, unsigned int i, node root)
{
	C[i] = root->c;
	t[i] = root->t;
	node lroot = root->left;
	if(lroot!=0)
	{
		huffmanTree->n_inode++;
		L[i] = huffmanTree->n_inode;
		pad_tree_uchar(huffmanTree, L,R,C,t, huffmanTree->n_inode, lroot);
	}
	node rroot = root->right;
	if(rroot!=0)
	{
		huffmanTree->n_inode++;
		R[i] = huffmanTree->n_inode;
		pad_tree_uchar(huffmanTree, L,R,C,t, huffmanTree->n_inode, rroot);
	}
}
void pad_tree_ushort(HuffmanTree* huffmanTree, unsigned short* L, unsigned short* R, unsigned int* C, unsigned char* t, unsigned int i, node root)
{
	C[i] = root->c;
	t[i] = root->t;
	node lroot = root->left;
	if(lroot!=0)
	{
		huffmanTree->n_inode++;
		L[i] = huffmanTree->n_inode;
		pad_tree_ushort(huffmanTree,L,R,C,t,huffmanTree->n_inode, lroot);
	}
	node rroot = root->right;
	if(rroot!=0)
	{
		huffmanTree->n_inode++;
		R[i] = huffmanTree->n_inode;
		pad_tree_ushort(huffmanTree,L,R,C,t,huffmanTree->n_inode, rroot);
	}
}
void pad_tree_uint(HuffmanTree* huffmanTree, unsigned int* L, unsigned int* R, unsigned int* C, unsigned char* t, unsigned int i, node root)
{
	C[i] = root->c;
	t[i] = root->t;
	node lroot = root->left;
	if(lroot!=0)
	{
		huffmanTree->n_inode++;
		L[i] = huffmanTree->n_inode;
		pad_tree_uint(huffmanTree,L,R,C,t,huffmanTree->n_inode, lroot);
	}
	node rroot = root->right;
	if(rroot!=0)
	{
		huffmanTree->n_inode++;
		R[i] = huffmanTree->n_inode;
		pad_tree_uint(huffmanTree,L,R,C,t,huffmanTree->n_inode, rroot);
	}
}
unsigned int convert_HuffTree_to_bytes_anyStates(HuffmanTree* huffmanTree, int nodeCount, unsigned char** out)
{
	if(nodeCount<=256)
	{
		unsigned char* L = (unsigned char*)malloc(nodeCount*sizeof(unsigned char));
		memset(L, 0, nodeCount*sizeof(unsigned char));
		unsigned char* R = (unsigned char*)malloc(nodeCount*sizeof(unsigned char));
		memset(R, 0, nodeCount*sizeof(unsigned char));
		unsigned int* C = (unsigned int*)malloc(nodeCount*sizeof(unsigned int));
		memset(C, 0, nodeCount*sizeof(unsigned int));
		unsigned char* t = (unsigned char*)malloc(nodeCount*sizeof(unsigned char));
		memset(t, 0, nodeCount*sizeof(unsigned char));

		pad_tree_uchar(huffmanTree,L,R,C,t,0,huffmanTree->qq[1]);

		unsigned int totalSize = 1+3*nodeCount*sizeof(unsigned char)+nodeCount*sizeof(unsigned int);
		*out = (unsigned char*)malloc(totalSize*sizeof(unsigned char));
		(*out)[0] = (unsigned char)sysEndianType;
		memcpy(*out+1, L, nodeCount*sizeof(unsigned char));
		memcpy((*out)+1+nodeCount*sizeof(unsigned char),R,nodeCount*sizeof(unsigned char));
		memcpy((*out)+1+2*nodeCount*sizeof(unsigned char),C,nodeCount*sizeof(unsigned int));
		memcpy((*out)+1+2*nodeCount*sizeof(unsigned char)+nodeCount*sizeof(unsigned int), t, nodeCount*sizeof(unsigned char));
		free(L);
		free(R);
		free(C);
		free(t);
		return totalSize;

	}
	else if(nodeCount<=65536)
	{
		unsigned short* L = (unsigned short*)malloc(nodeCount*sizeof(unsigned short));
		memset(L, 0, nodeCount*sizeof(unsigned short));
		unsigned short* R = (unsigned short*)malloc(nodeCount*sizeof(unsigned short));
		memset(R, 0, nodeCount*sizeof(unsigned short));
		unsigned int* C = (unsigned int*)malloc(nodeCount*sizeof(unsigned int));
		memset(C, 0, nodeCount*sizeof(unsigned int));
		unsigned char* t = (unsigned char*)malloc(nodeCount*sizeof(unsigned char));
		memset(t, 0, nodeCount*sizeof(unsigned char));
		pad_tree_ushort(huffmanTree,L,R,C,t,0,huffmanTree->qq[1]);
		unsigned int totalSize = 1+2*nodeCount*sizeof(unsigned short)+nodeCount*sizeof(unsigned char) + nodeCount*sizeof(unsigned int);
		*out = (unsigned char*)malloc(totalSize);
		(*out)[0] = (unsigned char)sysEndianType;
		memcpy(*out+1, L, nodeCount*sizeof(unsigned short));
		memcpy((*out)+1+nodeCount*sizeof(unsigned short),R,nodeCount*sizeof(unsigned short));
		memcpy((*out)+1+2*nodeCount*sizeof(unsigned short),C,nodeCount*sizeof(unsigned int));
		memcpy((*out)+1+2*nodeCount*sizeof(unsigned short)+nodeCount*sizeof(unsigned int),t,nodeCount*sizeof(unsigned char));
		free(L);
		free(R);
		free(C);
		free(t);
		return totalSize;
	}
	else //nodeCount>65536
	{
		unsigned int* L = (unsigned int*)malloc(nodeCount*sizeof(unsigned int));
		memset(L, 0, nodeCount*sizeof(unsigned int));
		unsigned int* R = (unsigned int*)malloc(nodeCount*sizeof(unsigned int));
		memset(R, 0, nodeCount*sizeof(unsigned int));
		unsigned int* C = (unsigned int*)malloc(nodeCount*sizeof(unsigned int));
		memset(C, 0, nodeCount*sizeof(unsigned int));
		unsigned char* t = (unsigned char*)malloc(nodeCount*sizeof(unsigned char));
		memset(t, 0, nodeCount*sizeof(unsigned char));
		pad_tree_uint(huffmanTree, L,R,C,t,0,huffmanTree->qq[1]);

		//debug
		//node root = new_node2(0,0);
		//unpad_tree_uint(L,R,C,t,0,root);

		unsigned int totalSize = 1+3*nodeCount*sizeof(unsigned int)+nodeCount*sizeof(unsigned char);
		*out = (unsigned char*)malloc(totalSize);
		(*out)[0] = (unsigned char)sysEndianType;
		memcpy(*out+1, L, nodeCount*sizeof(unsigned int));
		memcpy((*out)+1+nodeCount*sizeof(unsigned int),R,nodeCount*sizeof(unsigned int));
		memcpy((*out)+1+2*nodeCount*sizeof(unsigned int),C,nodeCount*sizeof(unsigned int));
		memcpy((*out)+1+3*nodeCount*sizeof(unsigned int),t,nodeCount*sizeof(unsigned char));
		free(L);
		free(R);
		free(C);
		free(t);
		return totalSize;
	}
}

void unpad_tree_uchar(HuffmanTree* huffmanTree, unsigned char* L, unsigned char* R, unsigned int* C, unsigned char *t, unsigned int i, node root)
{
	//root->c = C[i];
	if(root->t==0)
	{
		unsigned char l, r;
		l = L[i];
		if(l!=0)
		{
			node lroot = new_node2(huffmanTree,C[l],t[l]);
			root->left = lroot;
			unpad_tree_uchar(huffmanTree,L,R,C,t,l,lroot);
		}
		r = R[i];
		if(r!=0)
		{
			node rroot = new_node2(huffmanTree,C[r],t[r]);
			root->right = rroot;
			unpad_tree_uchar(huffmanTree,L,R,C,t,r,rroot);
		}
	}
}

void unpad_tree_ushort(HuffmanTree* huffmanTree, unsigned short* L, unsigned short* R, unsigned int* C, unsigned char* t, unsigned int i, node root)
{
	//root->c = C[i];
	if(root->t==0)
	{
		unsigned short l, r;
		l = L[i];
		if(l!=0)
		{
			node lroot = new_node2(huffmanTree,C[l],t[l]);
			root->left = lroot;
			unpad_tree_ushort(huffmanTree,L,R,C,t,l,lroot);
		}
		r = R[i];
		if(r!=0)
		{
			node rroot = new_node2(huffmanTree,C[r],t[r]);
			root->right = rroot;
			unpad_tree_ushort(huffmanTree,L,R,C,t,r,rroot);
		}
	}
}

void unpad_tree_uint(HuffmanTree* huffmanTree, unsigned int* L, unsigned int* R, unsigned int* C, unsigned char* t, unsigned int i, node root)
{
	//root->c = C[i];
	if(root->t==0)
	{
		unsigned int l, r;
		l = L[i];
		if(l!=0)
		{
			node lroot = new_node2(huffmanTree,C[l],t[l]);
			root->left = lroot;
			unpad_tree_uint(huffmanTree,L,R,C,t,l,lroot);
		}
		r = R[i];
		if(r!=0)
		{
			node rroot = new_node2(huffmanTree,C[r],t[r]);
			root->right = rroot;
			unpad_tree_uint(huffmanTree,L,R,C,t,r,rroot);
		}
	}
}

node reconstruct_HuffTree_from_bytes_anyStates(HuffmanTree *huffmanTree, unsigned char* bytes, int nodeCount)
{
	if(nodeCount<=256)
	{
		unsigned char* L = (unsigned char*)malloc(nodeCount*sizeof(unsigned char));
		memset(L, 0, nodeCount*sizeof(unsigned char));
		unsigned char* R = (unsigned char*)malloc(nodeCount*sizeof(unsigned char));
		memset(R, 0, nodeCount*sizeof(unsigned char));
		unsigned int* C = (unsigned int*)malloc(nodeCount*sizeof(unsigned int));
		memset(C, 0, nodeCount*sizeof(unsigned int));
		unsigned char* t = (unsigned char*)malloc(nodeCount*sizeof(unsigned char));
		memset(t, 0, nodeCount*sizeof(unsigned char));
		unsigned char cmpSysEndianType = bytes[0];
		if(cmpSysEndianType!=(unsigned char)sysEndianType)
		{
			unsigned char* p = (unsigned char*)(bytes+1+2*nodeCount*sizeof(unsigned char));
			size_t i = 0, size = nodeCount*sizeof(unsigned int);
			while(1)
			{
				symTransform_4bytes(p);
				i+=sizeof(unsigned int);
				if(i<size)
					p+=sizeof(unsigned int);
				else
					break;
			}
		}
		memcpy(L, bytes+1, nodeCount*sizeof(unsigned char));
		memcpy(R, bytes+1+nodeCount*sizeof(unsigned char), nodeCount*sizeof(unsigned char));
		memcpy(C, bytes+1+2*nodeCount*sizeof(unsigned char), nodeCount*sizeof(unsigned int));
		memcpy(t, bytes+1+2*nodeCount*sizeof(unsigned char)+nodeCount*sizeof(unsigned int), nodeCount*sizeof(unsigned char));
		node root = new_node2(huffmanTree, C[0],t[0]);
		unpad_tree_uchar(huffmanTree,L,R,C,t,0,root);
		free(L);
		free(R);
		free(C);
		free(t);
		return root;
	}
	else if(nodeCount<=65536)
	{
		unsigned short* L = (unsigned short*)malloc(nodeCount*sizeof(unsigned short));
		memset(L, 0, nodeCount*sizeof(unsigned short));
		unsigned short* R = (unsigned short*)malloc(nodeCount*sizeof(unsigned short));
		memset(R, 0, nodeCount*sizeof(unsigned short));
		unsigned int* C = (unsigned int*)malloc(nodeCount*sizeof(unsigned int));
		memset(C, 0, nodeCount*sizeof(unsigned int));
		unsigned char* t = (unsigned char*)malloc(nodeCount*sizeof(unsigned char));
		memset(t, 0, nodeCount*sizeof(unsigned char));

		unsigned char cmpSysEndianType = bytes[0];
		if(cmpSysEndianType!=(unsigned char)sysEndianType)
		{
			unsigned char* p = (unsigned char*)(bytes+1);
			size_t i = 0, size = 2*nodeCount*sizeof(unsigned short);

			while(1)
			{
				symTransform_2bytes(p);
				i+=sizeof(unsigned short);
				if(i<size)
					p+=sizeof(unsigned short);
				else
					break;
			}

			size = nodeCount*sizeof(unsigned int);
			while(1)
			{
				symTransform_4bytes(p);
				i+=sizeof(unsigned int);
				if(i<size)
					p+=sizeof(unsigned int);
				else
					break;
			}
		}

		memcpy(L, bytes+1, nodeCount*sizeof(unsigned short));
		memcpy(R, bytes+1+nodeCount*sizeof(unsigned short), nodeCount*sizeof(unsigned short));
		memcpy(C, bytes+1+2*nodeCount*sizeof(unsigned short), nodeCount*sizeof(unsigned int));

		memcpy(t, bytes+1+2*nodeCount*sizeof(unsigned short)+nodeCount*sizeof(unsigned int), nodeCount*sizeof(unsigned char));

		node root = new_node2(huffmanTree,0,0);
		unpad_tree_ushort(huffmanTree,L,R,C,t,0,root);
		free(L);
		free(R);
		free(C);
		free(t);
		return root;
	}
	else //nodeCount>65536
	{
		unsigned int* L = (unsigned int*)malloc(nodeCount*sizeof(unsigned int));
		memset(L, 0, nodeCount*sizeof(unsigned int));
		unsigned int* R = (unsigned int*)malloc(nodeCount*sizeof(unsigned int));
		memset(R, 0, nodeCount*sizeof(unsigned int));
		unsigned int* C = (unsigned int*)malloc(nodeCount*sizeof(unsigned int));
		memset(C, 0, nodeCount*sizeof(unsigned int));
		unsigned char* t = (unsigned char*)malloc(nodeCount*sizeof(unsigned char));
		memset(t, 0, nodeCount*sizeof(unsigned char));
		unsigned char cmpSysEndianType = bytes[0];
		if(cmpSysEndianType!=(unsigned char)sysEndianType)
		{
			unsigned char* p = (unsigned char*)(bytes+1);
			size_t i = 0, size = 3*nodeCount*sizeof(unsigned int);
			while(1)
			{
				symTransform_4bytes(p);
				i+=sizeof(unsigned int);
				if(i<size)
					p+=sizeof(unsigned int);
				else
					break;
			}
		}

		memcpy(L, bytes+1, nodeCount*sizeof(unsigned int));
		memcpy(R, bytes+1+nodeCount*sizeof(unsigned int), nodeCount*sizeof(unsigned int));
		memcpy(C, bytes+1+2*nodeCount*sizeof(unsigned int), nodeCount*sizeof(unsigned int));

		memcpy(t, bytes+1+3*nodeCount*sizeof(unsigned int), nodeCount*sizeof(unsigned char));

		node root = new_node2(huffmanTree,0,0);
		unpad_tree_uint(huffmanTree,L,R,C,t,0,root);
		free(L);
		free(R);
		free(C);
		free(t);
		return root;
	}
}
void encode_withEPAQ(int* s, size_t length, unsigned char* out, size_t* outSize) {
	printf("Quantization Factor Size: {%ld}\n", length);
	unsigned char* new_out;
	lpaq_compress(s, length, 3, &new_out, outSize, 0);
	memcpy(out, new_out, *outSize);
	free(new_out);
	return;
}
void encode_withTree(HuffmanTree* huffmanTree, int *s, size_t length, unsigned char **out, size_t *outSize)
{
	printf("Quantization Factor Size: {%ld}\n", length);
	if(USE_LPAQ && (USE_BITPACK == 0)) {
		lpaq_compress(s, length, LPAQ_LEVEL, out, outSize, 0);
	} else if(USE_LPAQ && USE_BITPACK) {
		// bitpacking s 
		int base, bitwidth, bitlength;
		unsigned int* bitpackbits;
		bitpack(s, length, &bitpackbits, &bitlength, &bitwidth, &base);
		// printf("originallength = %d, bitlength = %d, bitwidth = %d, base = %d\n", length, bitlength, bitwidth, base);
		// store base, bitwidth and bitlength, which is sizeof(int) * 3 = 12 bytes
		unsigned char* lpaqbits;
		lpaq_compress((int *)bitpackbits, bitlength, LPAQ_LEVEL, &lpaqbits, outSize, bitwidth);
		*outSize += 12;
		*out = (unsigned char*)malloc(*outSize);
		char buffer[4];
		intToBytes_bigEndian(buffer, base);
		memcpy(*out, buffer, 4);
		intToBytes_bigEndian(buffer, bitwidth);
		memcpy(*out+4, buffer, 4);
		intToBytes_bigEndian(buffer, bitlength);
		memcpy(*out+8, buffer, 4);
		memcpy(*out+12, lpaqbits, *outSize-12);
		free(lpaqbits);
		free(bitpackbits);
	} else if((USE_LPAQ == 0) && USE_BITPACK) {
		int base, bitwidth, bitlength;
		unsigned int* bitpackbits;
		bitpack(s, length, &bitpackbits, &bitlength, &bitwidth, &base);
		*out = (unsigned char*)malloc(bitlength * (sizeof(unsigned int) / sizeof(unsigned char)) + 12);
		char buffer[4];
		intToBytes_bigEndian(buffer, base);
		memcpy(*out, buffer, 4);
		intToBytes_bigEndian(buffer, bitwidth);
		memcpy(*out+4, buffer, 4);
		intToBytes_bigEndian(buffer, bitlength);
		memcpy(*out+8, buffer, 4);
		memcpy(*out+12, bitpackbits, bitlength * (sizeof(unsigned int) / sizeof(unsigned char)));
		*outSize = bitlength * (sizeof(unsigned int) / sizeof(unsigned char)) + 12;
		free(bitpackbits);
	} else {
		size_t i;
		int nodeCount = 0;
		unsigned char *treeBytes, buffer[4];

		init(huffmanTree, s, length);
		for (i = 0; i < huffmanTree->stateNum; i++)
			if (huffmanTree->code[i]) nodeCount++;
		nodeCount = nodeCount*2-1;
		unsigned int treeByteSize = convert_HuffTree_to_bytes_anyStates(huffmanTree,nodeCount, &treeBytes);

		*out = (unsigned char*)malloc(length*sizeof(int)+treeByteSize);
		intToBytes_bigEndian(buffer, nodeCount);
		memcpy(*out, buffer, 4);
		intToBytes_bigEndian(buffer, huffmanTree->stateNum/2); //real number of intervals
		memcpy(*out+4, buffer, 4);
		memcpy(*out+8, treeBytes, treeByteSize);
		free(treeBytes);
		size_t enCodeSize = 0;
		encode(huffmanTree, s, length, *out+8+treeByteSize, &enCodeSize);
		*outSize = 8+treeByteSize+enCodeSize;
	#if HAVE_WRITESTATS
		writeHuffmanInfo(treeByteSize, enCodeSize, 0, nodeCount);
	#endif
	}
	printf("Quantization Factor Compressed Size: {%ld}\n", *outSize);
}

int encode_withTree_MSST19(HuffmanTree* huffmanTree, int *s, size_t length, unsigned char **out, size_t *outSize)
{
	printf("|encode_withTree_MSST19|\n");
	//struct ClockPoint clockPointInit;
	//TimeDurationStart("init", &clockPointInit);
	size_t i;
	int nodeCount = 0;
	unsigned char *treeBytes, buffer[4];

	init(huffmanTree, s, length);

	int maxBits = 0;
	for (i = 0; i < huffmanTree->stateNum; i++)
		if (huffmanTree->code[i]){
			nodeCount++;
			if(huffmanTree->cout[i] > maxBits) maxBits = huffmanTree->cout[i];
		}
	nodeCount = nodeCount*2-1;
	//TimeDurationEnd(&clockPointInit);
	//struct ClockPoint clockPointST;
	//TimeDurationStart("save tree", &clockPointST);
	unsigned int treeByteSize = convert_HuffTree_to_bytes_anyStates(huffmanTree,nodeCount, &treeBytes);
	//printf("treeByteSize = %d\n", treeByteSize);

	*out = (unsigned char*)malloc(length*sizeof(int)+treeByteSize);
	intToBytes_bigEndian(buffer, nodeCount);
	memcpy(*out, buffer, 4);
	intToBytes_bigEndian(buffer, huffmanTree->stateNum/2); //real number of intervals
	memcpy(*out+4, buffer, 4);
	memcpy(*out+8, treeBytes, treeByteSize);
	free(treeBytes);
	size_t enCodeSize = 0;
	//TimeDurationEnd(&clockPointST);
	//struct ClockPoint clockPointEncode;
	//TimeDurationStart("encode", &clockPointEncode);
	encode(huffmanTree, s, length, *out+8+treeByteSize, &enCodeSize);
	*outSize = 8+treeByteSize+enCodeSize;
	//TimeDurationEnd(&clockPointEncode);
	//unsigned short state[length];
	//decode(*out+4+treeByteSize, enCodeSize, qqq[0], state);
	//printf("dataSeriesLength=%d",length );
	return maxBits;
}
void decode_withEPAQ(unsigned char *s, size_t targetLength, int *out) {
	lpaq_decompress(s, targetLength, out, targetLength);
}

/**
 * @par *out rememmber to allocate targetLength short_type data for it beforehand.
 *
 * */
void decode_withTree(HuffmanTree* huffmanTree, unsigned char *s, size_t targetLength, int *out)
{
	printf("|decode_withTree|, length: %ld\n", targetLength);
	if(USE_LPAQ && (USE_BITPACK==0)) {
		lpaq_decompress(s, targetLength, out, targetLength);
	}else if(USE_LPAQ && USE_BITPACK){
		// bitunpack s
		int base, bitwidth, bitlength;
		base = bytesToInt_bigEndian(s);
		bitwidth = bytesToInt_bigEndian(s+4);
		bitlength = bytesToInt_bigEndian(s+8);
		printf("bitwidth:%d, bitlength:%d \n", bitwidth, bitlength);
		unsigned char* lpaqbits = (unsigned char*)(s+12);
		unsigned int* bitpackbits = (unsigned int*)malloc(bitlength*sizeof(unsigned int));
		lpaq_decompress(lpaqbits, bitlength, (int*)bitpackbits, bitwidth);
		bitunpack(bitpackbits, bitlength, bitwidth, base, out, targetLength);
	} else if((USE_LPAQ == 0) && USE_BITPACK) {
		int base, bitwidth, bitlength;
		base = bytesToInt_bigEndian(s);
		bitwidth = bytesToInt_bigEndian(s+4);
		bitlength = bytesToInt_bigEndian(s+8);
		printf("base: %d, bitlength: %d, bitwidth: %d\n", base, bitlength, bitwidth);
		bitunpack((unsigned int*)(s+12), bitlength, bitwidth, base, out, targetLength);
		// for(int i = 0; i < targetLength; i++) {
		// 	printf("%d ", out[i]);
		// } 
		// printf("\n");
		// for(int i = 0; i < bitlength; i++) {
		// 	printf("%u ", ((unsigned int*)(s+12))[i]);
		// } 
		// printf("\n");
	} else {
		size_t encodeStartIndex;
		size_t nodeCount = bytesToInt_bigEndian(s);
		node root = reconstruct_HuffTree_from_bytes_anyStates(huffmanTree,s+8, nodeCount);

		//sdi: Debug
		/*	build_code(root, 0, 0, 0);
		int i;
		uint64_t code_1, code_2;
		for (i = 0; i < stateNum; i++)
			if (code[i])
			{
				printf("%d: %lu,%lu ; %u\n", i, (code[i])[0],(code[i])[1], cout[i]);
				//code_1 = (code[i])[0];
			}*/

		if(nodeCount<=256)
			encodeStartIndex = 1+3*nodeCount*sizeof(unsigned char)+nodeCount*sizeof(unsigned int);
		else if(nodeCount<=65536)
			encodeStartIndex = 1+2*nodeCount*sizeof(unsigned short)+nodeCount*sizeof(unsigned char)+nodeCount*sizeof(unsigned int);
		else
			encodeStartIndex = 1+3*nodeCount*sizeof(unsigned int)+nodeCount*sizeof(unsigned char);
		decode(s+8+encodeStartIndex, targetLength, root, out);
	}
}

void decode_withTree_MSST19(HuffmanTree* huffmanTree, unsigned char *s, size_t targetLength, int *out, int maxBits)
{
	printf("|decode_withTree_MSST19|\n");
	size_t encodeStartIndex;
	size_t nodeCount = bytesToInt_bigEndian(s);
	node root = reconstruct_HuffTree_from_bytes_anyStates(huffmanTree,s+8, nodeCount);

	//sdi: Debug
/*	build_code(root, 0, 0, 0);
	int i;
	uint64_t code_1, code_2;
	for (i = 0; i < stateNum; i++)
		if (code[i])
		{
			printf("%d: %lu,%lu ; %u\n", i, (code[i])[0],(code[i])[1], cout[i]);
			//code_1 = (code[i])[0];
		}*/

	if(nodeCount<=256)
		encodeStartIndex = 1+3*nodeCount*sizeof(unsigned char)+nodeCount*sizeof(unsigned int);
	else if(nodeCount<=65536)
		encodeStartIndex = 1+2*nodeCount*sizeof(unsigned short)+nodeCount*sizeof(unsigned char)+nodeCount*sizeof(unsigned int);
	else
		encodeStartIndex = 1+3*nodeCount*sizeof(unsigned int)+nodeCount*sizeof(unsigned char);

	decode_MSST19(s+8+encodeStartIndex, targetLength, root, out, maxBits);
}

void SZ_ReleaseHuffman(HuffmanTree* huffmanTree)
{
	size_t i;
	free(huffmanTree->pool);
	huffmanTree->pool = NULL;
	free(huffmanTree->qqq);
	huffmanTree->qqq = NULL;
	for(i=0;i<huffmanTree->stateNum;i++)
	{
		if(huffmanTree->code[i]!=NULL)
			free(huffmanTree->code[i]);
	}
	free(huffmanTree->code);
	huffmanTree->code = NULL;
	free(huffmanTree->cout);
	huffmanTree->cout = NULL;
	free(huffmanTree);
	huffmanTree = NULL;
}
