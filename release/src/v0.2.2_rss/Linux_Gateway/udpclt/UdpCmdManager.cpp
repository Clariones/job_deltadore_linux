#include "UdpCmdManager.h"
#include <stdio.h>
#include <string.h>

#include "CmdControlLightColorHandler.h"
#include "CmdControlLightLevelHandler.h"
#include "CmdControlLightHandler.h"
#include "CmdControlRollerShutterHandler.h"
#include "CmdDeleteNodeHandler.h"
#include "CmdForceRefreshTopologyHandler.h"
#include "CmdGetTopologyHandler.h"
#include "CmdQueryLightColorHandler.h"
#include "CmdQueryLightInfoHandler.h"
#include "CmdQueryLightStatusHandler.h"
#include "CmdQueryRollerShutterInfoHandler.h"
#include "CmdQueryRollerShutterStatusHandler.h"
#include "CmdRegisterNodeHandler.h"
#include "CmdSetNodeNameHandler.h"
#include "CmdSetNodeDeviceTypeHandler.h"

#define BUFF_SIZE (10*1024)

#define HANDLER(name) \
    handlers[i++] = new name();                                                  \
    if (i >= MAX_HANDLERS_NUMBER){                                               \
        logger(LEVEL_ERROR, "Can only have %d handlers\n", MAX_HANDLERS_NUMBER); \
        exit(-1);                                                                \
    }else{                                                                       \
        logger(LEVEL_INFO, "WebUI command handler " #name " created\n");           \
    }

static const char* RESP(const char* pStr){
    char* pBuf = new char[strlen(pStr)+1];
    strcpy(pBuf, pStr);
    return pBuf;
}

UdpCmdManager::UdpCmdManager()
{
    int i = 0;
    threadId = 0;
    pTcpServer = NULL;
    recvBuffer = NULL;
    HANDLER(CmdControlLightColorHandler);
    HANDLER(CmdControlLightLevelHandler);
    HANDLER(CmdControlLightHandler);
    HANDLER(CmdControlRollerShutterHandler);
    HANDLER(CmdDeleteNodeHandler);
    HANDLER(CmdForceRefreshTopologyHandler);
    HANDLER(CmdGetTopologyHandler);
    HANDLER(CmdQueryLightColorHandler);
    HANDLER(CmdQueryLightInfoHandler);
    HANDLER(CmdQueryLightStatusHandler);
    HANDLER(CmdQueryRollerShutterInfoHandler);
    HANDLER(CmdQueryRollerShutterStatusHandler);
    HANDLER(CmdRegisterNodeHandler);
    HANDLER(CmdSetNodeNameHandler);
    HANDLER(CmdSetNodeDeviceTypeHandler);
//    handlers[i++] = new CmdControlLightHandler();
//    if (i >= MAX_HANDLERS_NUMBER){
//        printf("Can only have %d handlers\n", MAX_HANDLERS_NUMBER);
//        exit(-1);
//    }
    handlers[i++] = NULL;
}

UdpCmdManager::~UdpCmdManager()
{
    //dtor
}
void UdpCmdManager::setPort(int pPort)
{
    port = pPort;
}

int UdpCmdManager::getPort()
{
    return port;
}

pthread_t UdpCmdManager::start()
{
    runningFlag = true;
    pTcpServer = new TcpServer();
    pTcpServer->listening(port);
    recvBuffer = new char[BUFF_SIZE];
    pthread_create (&threadId, NULL, &startRoutine, this);

    return threadId;
}

void UdpCmdManager::run(){
    printf("TCP command listener is running...\n");
    char* commandName = new char[1500];
    char* resultData = new char[BUFF_SIZE];
    ConnectionData data;

    while(runningFlag){

        // wait client to connect
        int result = pTcpServer->waitConnect(&data, 1000);
        if (result <= 0){
            continue;
        }

        // if connected, read from client
        int pos = 0;
        bool gotData = false;
        int retryTimes = 0;
        int recvLen;
        do {
            recvLen = pTcpServer->read(&data,(void*)recvBuffer,BUFF_SIZE, 500);
            printf("received %d bytes\n", recvLen);
            if (recvLen > 0) {
                gotData = true;
                char* target = resultData + pos;
                memcpy(((void*)target), recvBuffer, recvLen);
                pos += recvLen;
            }else{
                retryTimes++;
            }
        } while(retryTimes < 10 && (recvLen > 0 || !gotData));
        resultData[pos] = 0;

        printf("Got message: %s\n", resultData);
        sscanf(resultData, "%s", commandName);

        if (strcmp("exit", commandName) == 0){
            sendResponse(RESP("{\"message\":\"success\", \"success\":true}"), &data);
            this->stop();
            break;
        }
        if (strcmp("help", commandName) == 0){
            sendResponse(getHelpMessage(resultData), &data);
            continue;
        }

        printf("Command name is %s\n", commandName);
        processCommand(commandName, resultData, &data);
    }
    delete commandName;
    delete resultData;
}
const char* UdpCmdManager::getHelpMessage(const char* pCmdInput)
{
    if (strcmp("help", pCmdInput) == 0){
        // only help. not help for a topic
        char* pResult = new char[MAX_UDP_SIZE+1];
        int pos = sprintf(pResult, "Below commands are valid:\n");
        char* pBuff = pResult + pos;
        for(int i=0;i<MAX_HANDLERS_NUMBER;i++){
            BaseCmdHandler* pHandler = handlers[i];
            if (pHandler == NULL){
                return pResult;
            }
            pos += sprintf(pBuff, "  %s\n", pHandler->getCommandName());
            pBuff = pResult + pos;
        }
        return RESP("You got wrong version. Please contact technical support.");
    }
    char* cmdName = new char[strlen(pCmdInput)];
    sscanf(pCmdInput+5, "%s", cmdName);
    for(int i=0;i<MAX_HANDLERS_NUMBER;i++){
        BaseCmdHandler* pHandler = handlers[i];
        if (pHandler == NULL){
            delete cmdName;
            return RESP("Unknow command, please use help to get valid commands first.");
        }

        if (strcmp(cmdName, pHandler->getCommandName()) == 0){
            delete cmdName;
            return RESP(pHandler->getUsage());
        }
    }
    delete cmdName;
    return RESP("You got wrong version. Please contact technical support.");
}

void UdpCmdManager::sendResponse(const char* pResponse, const ConnectionData* pRemoteArg)
{
    printf("Message response is: %s\n", pResponse);
    pTcpServer->write(pRemoteArg, pResponse, strlen(pResponse));
    delete pResponse;
}

const char* UdpCmdManager::responseCommandUnknown(const char* pCmdName)
{
    char* pData = new char[MAX_UDP_SIZE+1];
    sprintf(pData, "Command %s unknown. Try help for valid command list", pCmdName);
    return pData;
}

void UdpCmdManager::processCommand(const char* pCmdName, const char* pCmdInput, const ConnectionData* pRemoteNode)
{
    for(int i=0;i<MAX_HANDLERS_NUMBER;i++){
        BaseCmdHandler* pHandler = handlers[i];
        if (pHandler == NULL){
            sendResponse(responseCommandUnknown(pCmdName), pRemoteNode);
            return;
        }

        if (strcmp(pCmdName, pHandler->getCommandName()) == 0){
            sendResponse(pHandler->handle(pCmdInput, getDriver()), pRemoteNode);
            return;
        }
    }
}

void UdpCmdManager::stop()
{
    printf("Set running flag to false\n");
    runningFlag = false;
}

void UdpCmdManager::release()
{
    if (threadId > 0) {
        pthread_join(threadId, NULL);
        threadId = 0;
    }

    if (pTcpServer != NULL) {
        pTcpServer->stop();
        delete pTcpServer;
    }

    if (recvBuffer != NULL) {
        delete recvBuffer;
        recvBuffer = NULL;
    }
}
