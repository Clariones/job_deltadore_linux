#include "TcpServer.h"



TcpServer::TcpServer()
{
    memset(client_fd,0,sizeof(client_fd));
}

TcpServer::~TcpServer()
{
    //dtor
}

int TcpServer::listening(int portNumber)
{
    int err;

    sock_port = portNumber;
    sock_fd = socket(AF_INET,SOCK_STREAM,IPPROTO_IP);
    if(sock_fd < 0){
        perror("socket:");
        return -1;
    }else{
        printf("TCP socket %d created at port %d\n", sock_fd, portNumber);
    }

    int on = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    //设为非阻塞
    if (fcntl(sock_fd, F_SETFL, O_NONBLOCK) == -1) {
        perror("set non-block:");
        return -3;
    }

        //初始化地址结构
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;           //协议族
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);   //本地地址
    server_addr.sin_port = htons(portNumber);

    err = bind(sock_fd,(struct sockaddr *)&server_addr,sizeof(server_addr));
    if(err<0)
    {
        perror("server : bind error:");
        return -5;
    }

    err = listen(sock_fd,BACKLOG);   //设置监听的队列大小
    if(err < 0)
    {
        perror("listen error:");
        return -7;
    }

    return 0;
}

int TcpServer::waitConnect(ConnectionData* pConnData, int timeout){
    struct timeval tv;
    socklen_t socklen = sizeof(SOCK_ADDR);

    // set timeout in millonseconds
    tv.tv_sec = timeout/1000;
    tv.tv_usec = (timeout % 1000)*1000;

    fd_set watchset;
    FD_ZERO(&watchset);
    FD_SET(sock_fd, &watchset);
    for (int i = 0; i < BACKLOG; i++) {
        if (client_fd[i] != 0) {
            FD_SET(client_fd[i], &watchset);
        }
    }

    int ret = select(sock_fd + 1, &watchset, NULL, NULL, &tv);
    if (ret < 0){
        perror("select:");
        return -1;
    }

    if (ret == 0){
        return 0;
    }

    if (!FD_ISSET(sock_fd, &watchset)) { //new connection
        return 0;
    }

    int new_cli_fd = accept(sock_fd,  (struct sockaddr *)(&(pConnData->clientSockAddr)), &socklen);
    if (new_cli_fd < 0){
        perror("accept:");
        return -1;
    }
    pConnData->clientSockFD = new_cli_fd;
    return 1;
}

void TcpServer::stop(){
    close(sock_fd);
}

int TcpServer::read(const ConnectionData* pConnData, void* buff, int maxSize, int timeoutMs){
    struct timeval timeout;
    timeout.tv_sec = timeoutMs/1000;
    timeout.tv_usec = (timeoutMs % 1000)*1000;
    int ret=setsockopt(pConnData->clientSockFD,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));
    if (ret < 0){
        perror("set recv timeout");
        return -1;
    }
    return recv(pConnData->clientSockFD, buff,maxSize,0);
}

int TcpServer::write(const ConnectionData* pConnData, const void* buff, int maxSize){
    return send(pConnData->clientSockFD, buff,maxSize,0);
}
