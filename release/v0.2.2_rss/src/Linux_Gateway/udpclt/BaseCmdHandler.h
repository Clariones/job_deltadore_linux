#ifndef BASECMDHANDLER_H
#define BASECMDHANDLER_H

#include <stdio.h>
#include "driver/DeltaDoreX2Driver.h"
#include "utils/cJSON.h"

#define MAX_UDP_SIZE (100*1024)

typedef const char* CMD_RETURN_TYPE;

class BaseCmdHandler
{
    public:
        BaseCmdHandler();
        virtual ~BaseCmdHandler();

    public:
        virtual const char * handle(const char* pCmd, DeltaDoreX2Driver* pDriver) = 0;
        virtual const char * getCommandName() = 0;
        virtual const char * getUsage() = 0;

    protected:
        virtual CMD_RETURN_TYPE getNextParamStartPosition(const char * pCmd);
        virtual bool isValidNetwork(int networkId) { return networkId >=0 && networkId < 12;};
        virtual bool isValidNode(int nodeId) { return nodeId >= 0 && nodeId < 16;};
        virtual bool isValidIntParam(int value, int lowLmt, int highLmt) { return value >= lowLmt && value <= highLmt;};
        virtual bool isValidStringParam(const char* value, int length);
        virtual CMD_RETURN_TYPE newMissingRequiredParametersResponse();
        virtual CMD_RETURN_TYPE newWrongIntParamResponse(const char* pMsg, int arg);
        virtual CMD_RETURN_TYPE newWrongStringParamResponse(const char* pMsg, char* pArg);
        virtual CMD_RETURN_TYPE newResponse(cJSON* pResponse);
        virtual void getParamInt(const char* pInput, int* pArg) {sscanf(pInput, "%d", pArg);};
        virtual void getParamString(const char* pInput, char* pArg) {sscanf(pInput, "%s", pArg);};

    private:
};

#endif // BASECMDHANDLER_H
