#ifndef _SZ_LPAQ
#define _SZ_LPAQ
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define USING_EPAQ

#ifdef USING_EPAQ
typedef unsigned char U8;
void epaqdecompress(int memLevel, int inlength, U8* inputbits, U8* outputbits, int outlength, int bitwidth);
void epaqcompress(int memLevel, int inlength, U8* inputbits, U8** outputbits, size_t* outlength, int bitwidth);

void lpaq_compress(int* content, size_t length, int level, unsigned char** output, size_t *output_size, int bitwidth) {
    // printf("lpaq_compress; original length: %d\n", length);
    epaqcompress(level, length, (U8 *)content, output, output_size, bitwidth);
    size_t real_output_size = *output_size + 9;
    unsigned char *buffer = (unsigned char*)malloc((int)real_output_size);
    intToBytes_bigEndian(buffer, *output_size + 1);
    intToBytes_bigEndian(buffer + 4, level);
    memcpy(buffer + 8, *output, *output_size);
    buffer[8 + *output_size] = EOF;
    free(*output);
    *output = buffer;
    U8* printer = (U8*)content;
    *output_size += 9;
        // for(int i = 0; i < *output_size; i++) {
        //     printf("%d ", content[i]);
        // }
}

void lpaq_decompress(unsigned char* content, size_t length, int* out, int bitwidth) {
    // printf("lpaq_decompress, length: %d\n", length);
    int inlength = bytesToInt_bigEndian(content);
    int level = bytesToInt_bigEndian(content + 4);
    printf("inlength: %d, level: %d\n", inlength, level);
    epaqdecompress(level, inlength, content + 8, (U8*)out, length * sizeof(int), bitwidth);
}
#else
void lpaq_compress(int* content, size_t length, int level, unsigned char** output, size_t *output_size, int t) {
    printf("lpaq compress, length: %d, level: %d\n", length, level);
    // unsigned char* data = (unsigned char*)malloc(length * 4);

	unsigned char buffer[4];
    // size_t i = 0;
    // for(i = 0; i < length;i++) {
    //   intToBytes_bigEndian(buffer, content[i]);
    //   // printf("%d ", content[i]);
    //   memcpy(data + i * 4, buffer, 4);
    // }
    // printf("\n"); 
    char tmp_input[] = "./sz_lpaq_tmp_input_com";
    char tmp_output[] = "./sz_lpaq_tmp_output_com";
    int tmp_fd = mkstemp(tmp_input);
    FILE* tmp_input_file = fdopen(tmp_fd, "w");
    fwrite(content, sizeof(char), 4 * length, tmp_input_file);
    fclose(tmp_input_file);

    char command[100];
    char* temp_filename = NULL;
    sprintf(command, "lpaq1 %d %s %s", level, tmp_input, tmp_output);
    system(command);


    FILE* tmp_output_file = fopen(tmp_output, "rb");
    fseek(tmp_output_file, 0, SEEK_END);
    *output_size = ftell(tmp_output_file);
    fseek(tmp_output_file, 0, SEEK_SET);
    *output = (unsigned char*)malloc(4 + *output_size);
    intToBytes_bigEndian(buffer, *output_size);
    memcpy(*output, buffer, 4);

    fread(*output+4, 1, *output_size, tmp_output_file);
    fclose(tmp_output_file);

    remove(tmp_input);
    remove(tmp_output);
    *output_size = *output_size + 4;
    unsigned char* printer = (unsigned char*)content;
}


void lpaq_decompress(unsigned char* content, size_t length, int* out, int t) {
    printf("lpaq decompress, length:%d\n", length);
    unsigned char buffer[4];
    memcpy(buffer, content, 4);
    int tmp_size = bytesToInt_bigEndian(buffer);
    char tmp_input[] = "./sz_lpaq_tmp_input_dec";
    char tmp_output[] = "./sz_lpaq_tmp_output_dec";

    int tmp_fd = mkstemp(tmp_input);
    FILE* tmp_input_file = fdopen(tmp_fd, "wb");
    fwrite(content+4, 1, tmp_size, tmp_input_file);
    fclose(tmp_input_file);
    char command[100];
    sprintf(command, "lpaq1 d %s %s", tmp_input, tmp_output);
    system(command);
    printf("%s\n", command);

    FILE* tmp_output_file = fopen(tmp_output, "rb");

    fread(out, sizeof(int), length, tmp_output_file);
    fclose(tmp_output_file);
    remove(tmp_input);
    remove(tmp_output);
}
#endif
#endif
