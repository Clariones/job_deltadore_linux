#include "Utils.h"



int openDevice(const char* devName, int options){
    return open(devName, options);
}

ssize_t readDevice (int fd, void *buf, size_t nbytes){
    ssize_t n = read(fd, buf, nbytes);
//    if (n > 0){
//        dumpHex("Read", buf, n);
//    }
    return n;
}
ssize_t writeDevice (int fd, const void *buf, size_t n){
    dumpHex("Send", buf, n);
    return write (fd, buf, n);
}
int closeDevice(int fd){
    return close(fd);
}

void dumpHex(const char* prefix, const void* pData, int size){
    const unsigned char* pByte = (const unsigned char*)pData;
    for(int i=0;i<size;i++){
        if (i % 32 == 0){
            if (i == 0){
                printf("%s:", prefix);
            }else{
                printf("\n%s ", prefix);
            }
        }else if (i % 16 == 0){
            printf("  ");
        }
        printf(" %02X", pByte[i]);
    }
    printf("\n");
}


int getNameLineCheckSum(const char* nameType, int network, int node, const char* nameString){
    return 0x55AA;
    int sum = 0x5A;
    int len = strlen(nameType);
    for(int i=0;i<len;i++){
        sum += (unsigned char) nameType[i];
    }
    sum += (0xFF ^ network);
    sum += (0xFF ^ node);
    len = strlen(nameString);
    for(int i=0;i<len;i++){
        sum += (unsigned char) nameString[i];
    }
    return sum & 0xFF;
}
