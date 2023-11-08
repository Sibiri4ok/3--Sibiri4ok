#include <stdio.h>
#include <string.h>
#include <stdint.h>
#define MAX_LEN_MESSAGE 256


int read_message(FILE *stream, void *buf) {
    int mSize = 0; // длина прочитываемого сообщения
    uint8_t result[MAX_LEN_MESSAGE]; // массив, в который записыаем прочитываемое сообщение
    int byte;
    for (int i = 0; i < MAX_LEN_MESSAGE; ++ i) {
        byte = getc(stream);
        if (byte==-1) break;
        result[i] = (uint8_t)byte;
        mSize ++;
    }
    int end_index = 0;
    for (int i = 0; i < MAX_LEN_MESSAGE; ++ i) printf("%x ",result[i]);
    puts("result end");
    int start_index;
    int array_numbers[] = {128,64,32,16,8,4,2,1};
    int countBite1 = 0;
    uint8_t find_end_marker = 0x00;
    
    uint16_t pair_start = (uint16_t)(result[0]<<8) | (uint16_t)(result[1]);
    int countReverseMarker = 0;
    int countStartReverseMarker = 0;
    int count_end_marker = 0;
    
    for (int i = 0; i < 2048; ++ i) { // проверка стартового маркера
        if (countStartReverseMarker%8==0) {
            pair_start = (uint16_t)(result[i/8]<<8) | (uint16_t)(result[(i/8)+1]);
        }
        if ( (uint8_t)(pair_start >> 8) == 0x7E ) {
            start_index = countStartReverseMarker + 8;
            break;
        }
        if ( !(((uint8_t)(pair_start >> 8)) & 128) ) {
            puts("37");
            fprintf(stderr, "Error, end-marker is not there");
            return EOF;
        }
        pair_start = pair_start << 1;
        countStartReverseMarker++;
    }
    for (int i = start_index; i < 2048; ++i) {
        find_end_marker = find_end_marker | ((result[i/8] & array_numbers[i%8])>0);
        if (find_end_marker == 0x7E) { 
            find_end_marker =  0x00;
            end_index = (end_index==0) ? i-8+1 : end_index;
            count_end_marker++;
        }
        find_end_marker = find_end_marker << 1;
    }
    printf("end_index = %d\n", end_index);
    if (count_end_marker == 1) {
        uint16_t pair = (uint16_t)(result[mSize-2]<<8) | (uint16_t)(result[mSize-1]);
        for (int i = 0; i < 8; ++ i) { // прверяем последний 
            if ((uint8_t)(pair>>i) == 0x7E) {
                end_index = mSize*8 - i - 8;
                break;
            }
        countReverseMarker++;
        }   
        if (countReverseMarker == 8) {
            puts("64");
            fprintf(stderr, "Error, end-marker is not there");
            return EOF;
        }
    }
    printf("end_index = %d\n", end_index);


    int countBit = 0;
    uint8_t buffer[MAX_LEN_MESSAGE]; // массив в который записываем декодированное сообщение

    for (int i = 0; i < MAX_LEN_MESSAGE; ++ i) buffer[i] = 0x00; // по дефолту он будет заполнен нулями

    for (int i = start_index; i < end_index + 1; ++ i) {
        buffer[countBit/8] = buffer[countBit/8] | ((result[i/8] & array_numbers[i%8])>0);
        if ((countBit%8)!=7) 
            buffer[countBit/8] = buffer[countBit/8] << 1;

        countBit ++;
        countBite1 = (result[i/8] & array_numbers[i%8]) ? countBite1+1: 0;
        if (countBite1 == 5) {
            i ++;
            if (result[i/8] & array_numbers[i%8]) {
                puts("84");
                fprintf(stderr, "The error is related to the payload");
                return EOF;
            }
            countBite1 = 0;
        }
    }
    countBit --;
    printf("countendmarker=%d\n", count_end_marker);
    printf("countBit=%d\n",countBit);
    if ( (countBit % 8) != 0 )  {
        puts("94");
        fprintf(stderr, "The byte is not whole");
        return EOF;
    }
    if (count_end_marker > 1) {
        countReverseMarker = 0;
        start_index = end_index + 17;
        uint16_t pair = (uint16_t)(result[mSize-2]<<8) | (uint16_t)(result[mSize-1]);
        for (int i = 0; i < 8; ++ i) { // прверяем последний 
            if ((uint8_t)(pair>>i) == 0x7E) {
                end_index = mSize*8 - i - 8;
                break;
            }
            countReverseMarker ++;
            if (countReverseMarker == 8) {
                puts("108");
                fprintf(stderr, "Error, end-marker is not there");
                return EOF;
            }
        }
        for (int i = start_index; i < end_index + 1; ++ i) {
            buffer[countBit/8] = buffer[countBit/8] | ((result[i/8] & array_numbers[i%8])>0);
            if ((countBit%8)!=7) 
                buffer[countBit/8] = buffer[countBit/8] << 1;
    
            countBit ++;
            countBite1 = (result[i/8] & array_numbers[i%8]) ? countBite1+1: 0;
            if (countBite1 == 5) {
                i ++;
                if (result[i/8] & array_numbers[i%8]) {
                    puts("124");
                    fprintf(stderr, "The error is related to the payload");
                    return EOF;
                }
                countBite1 = 0;
            }
        }
        countBit --;
    }
    if ( (countBit % 8) != 0 )  {
        puts("139");
        fprintf(stderr, "The byte is not whole");
        return EOF;
    }
    uint8_t *uinBuf = (uint8_t*)buf;
    for (int i = 0; i < countBit/8; ++ i) {
        uinBuf[i] = buffer[i];
    }
    return countBit/8;
}


int write_message(FILE* stream, const void *buf, size_t nbyte) {
    uint8_t message[MAX_LEN_MESSAGE];
    for (unsigned int i = 0; i < MAX_LEN_MESSAGE; ++ i){
        message[i] = (i <= nbyte) ? *((uint8_t *)buf + i): 0;
    }
    uint8_t result[MAX_LEN_MESSAGE];
    result[0] = 0x7E;
    uint8_t result_byte = 0;
    int count_bit = 0;
    int count_bit1 = 0;
    for (unsigned int i = 1;i < nbyte + 1; ++ i) {
        for (int index = 128; index >= 1; index/=2) {
            result_byte = result_byte | ((message[i - 1] & index)>0);
            count_bit ++;
            if (count_bit%8==0) {
                result[count_bit/8] = result_byte;
                result_byte = 0;
            }
            result_byte = result_byte << 1;
        
            count_bit1 = (message[i - 1] & (uint8_t)index) ? count_bit1 + 1:0;
            if (count_bit1 == 5) {
                count_bit++;
                count_bit1 = 0;
                if (count_bit%8==0) {
                    result[count_bit/8] = result_byte;
                    result_byte = 0;
                }
                result_byte = result_byte << 1;
            }
        }
    }
    result_byte = result_byte >> 1;
    if (count_bit%8==0) result[(count_bit/8) + 1] = 0x7E; 
    else {
        uint16_t pair = (uint16_t)(result_byte<<8) | (uint16_t)(0x7E);
        pair = pair << (8 - (count_bit%8));
        result[(count_bit/8)+1] = (uint8_t)(pair>>8);
        uint16_t pair1 = (uint16_t)(0x7E <<8) | (uint16_t)(255);
        pair1 = pair1 >> (count_bit%8);
        result[(count_bit/8)+2] = (uint8_t)(pair1);
    }
    int temp;
    for (int unsigned i = 0;i < 2 + nbyte + ((count_bit%8)>0); ++ i) {
        if (result[i]) {
            temp = putc(result[i], stream);
            if (temp == -1) {
                fprintf(stderr, "Fail with write");
                return EOF;
            }
        }
    }
    return nbyte;
}
