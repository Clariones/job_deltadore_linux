#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <stdio.h>
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

#define BACKLOG 1

typedef struct sockaddr_in SOCK_ADDR;

typedef struct {
    SOCK_ADDR clientSockAddr;
    int clientSockFD;
} ConnectionData;

class TcpServer
{
    public:
        TcpServer();
        virtual ~TcpServer();

        int listening(int portNumber);
        int waitConnect(ConnectionData* pConnData, int timeout);
        void stop();
        int read(const ConnectionData* pConnData, void* buff, int maxSize, int timeoutMs);
        int write(const ConnectionData* pConnData, const void* buff, int maxSize);
    protected:

    private:
        int sock_fd;
        int sock_port;
        SOCK_ADDR server_addr;
        int client_fd[BACKLOG];
};

#endif // TCPSERVER_H
