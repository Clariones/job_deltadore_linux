#include <iostream>
#include "TcpClient.h"
#include <stdlib.h>
#include <string.h>

#define SIZE (8*1024)

using namespace std;

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        printf("Usage:\n\t%s <ip_address> <port> <message>\nmust have message\n", argv[0]);
        return -1;
    }
//    cout << "Hello world!" << endl;

    TcpClient * pClient = new TcpClient();
    int ret = pClient->connectServer(argv[1], atoi(argv[2]));
    if (ret < 0){
        printf("{\"message\":\"connection failed\",\"code\":%d,\"success\":false}", ret);
        delete pClient;
        return -1;
    }
    //perror("COnnet");
    pClient->write(argv[3], strlen(argv[3])+1);
    //perror("Send");
    //printf("Send %s to server %s:%d\n", argv[3], argv[1], atoi(argv[2]));

    int recvLen;
    int pos = 0;
    bool gotData = false;
    char* buff = new char[SIZE];
    char* resultData = new char[128*1024];
    int retryTimes = 0;
    do {
        recvLen = pClient->read((void*)buff,SIZE, 500);
        //printf("received %d bytes\n", recvLen);
        if (recvLen > 0) {
            gotData = true;
            char* target = resultData + pos;
            memcpy(((void*)target), buff, recvLen);
            pos += recvLen;
        }else{
            retryTimes++;
        }
    } while(retryTimes < 240 && (recvLen > 0 || gotData == false));
    resultData[pos] = 0;

    pClient->stop();
    delete pClient;

    if (gotData){
        printf("%s", resultData);
    }else {
        printf("{\"message\":\"timeout\",\"success\":false}");
    }
    delete buff;
    delete resultData;
    return 0;
}
