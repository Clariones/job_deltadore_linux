#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/fcntl.h>

typedef struct sockaddr_in SOCK_ADDR;

class TcpClient
{
    public:
        TcpClient();
        virtual ~TcpClient();

        int connectServer(const char* pServerAddress, int port);
        int write(void* buff, int length);
        int read(void* buff, int maxLength, int timeoutMs);
        void stop();

    protected:

    private:
        SOCK_ADDR server_addr;
        int sock;
};

#endif // TCPCLIENT_H
