#include "TcpClient.h"

TcpClient::TcpClient()
{
    //ctor
}

TcpClient::~TcpClient()
{
    //dtor
}

int TcpClient::connectServer(const char* pServerAddress, int port){

//创建socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        //perror("socket Error!");
        return -2;
    }

    //填充sockaddr_in
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    int rtn = inet_pton(AF_INET, pServerAddress, &server_addr.sin_addr.s_addr);
    //或者是  serAddr.sin_addr.s_addr=inet_addr(serIP);
    if (rtn <= 0) {
        //perror("inet_pton Error!");
        return -4;
    }

    //链接服务器
    if (connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        //perror("connect Error!!\n");
        return -6;
    }

    return 0;
}

int TcpClient::write(void* buff, int length){
    return send(sock, buff, length, 0);
}
int TcpClient::read(void* buff, int maxLength, int timeoutMs){
    struct timeval timeout;
    timeout.tv_sec = timeoutMs/1000;
    timeout.tv_usec = (timeoutMs % 1000)*1000;
    int ret=setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));
    if (ret < 0){
        //perror("set recv timeout");
        return -1;
    }
    return recv(sock, buff,maxLength,0);
}

void TcpClient::stop(){
    close(sock);
}
