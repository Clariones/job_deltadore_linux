#include "BaseCmdHandler.h"

#define RESPONSE(format, args...)   \
    char* pResult = new char[MAX_UDP_SIZE+1];         \
    snprintf(pResult, MAX_UDP_SIZE, format, args);    \
    return pResult;


BaseCmdHandler::BaseCmdHandler()
{
    //ctor
}

BaseCmdHandler::~BaseCmdHandler()
{
    //dtor
}

const char* BaseCmdHandler::getNextParamStartPosition(const char* pCmd)
{
    const char* pChar = pCmd;
    char c;
    for(;;)
    {
        c = *pChar;
        if (c == 0)
        {
            return NULL;
        }
        if (c == ' ')
        {
            break;
        }
        pChar++;
    }
    for(;;)
    {
        c = *pChar;
        if (c == 0)
        {
            return NULL;
        }
        if (c != ' ')
        {
            break;
        }
        pChar++;
    }
    return pChar;
}

CMD_RETURN_TYPE BaseCmdHandler::newWrongIntParamResponse(const char* pMsg, int pArg)
{
//    RESPONSE(pMsg, pArg)
    char* pResult = new char[MAX_UDP_SIZE+1];
    snprintf(pResult, MAX_UDP_SIZE, pMsg, pArg);
    cJSON* root=cJSON_CreateObject();
    cJSON_AddStringToObject(root,"message",pResult);
    cJSON_AddBoolToObject(root,"success", false);
    char* pJson = cJSON_Print(root);
    strncpy(pResult, pJson, MAX_UDP_SIZE);
    cJSON_Delete(root);
    free(pJson);
    return pResult;
}

CMD_RETURN_TYPE BaseCmdHandler::newMissingRequiredParametersResponse()
{
    char* pResult = new char[MAX_UDP_SIZE+1];
    cJSON* root=cJSON_CreateObject();
    cJSON_AddStringToObject(root,"message","missing required parameters");
    cJSON_AddBoolToObject(root,"success", false);
    cJSON_AddStringToObject(root, "usage", getUsage());
    char* pJson = cJSON_Print(root);
    strncpy(pResult, pJson, MAX_UDP_SIZE);
    cJSON_Delete(root);
    free(pJson);
    return pResult;
}

CMD_RETURN_TYPE BaseCmdHandler::newWrongStringParamResponse(const char* pMsg, char* pArg)
{
    char* pResult = new char[MAX_UDP_SIZE+1];
    snprintf(pResult, MAX_UDP_SIZE, pMsg, pArg);
    cJSON* root=cJSON_CreateObject();
    cJSON_AddStringToObject(root,"message",pResult);
    cJSON_AddBoolToObject(root,"success", false);
    char* pJson = cJSON_Print(root);
    strncpy(pResult, pJson, MAX_UDP_SIZE);
    cJSON_Delete(root);
    free(pJson);
    return pResult;
}

CMD_RETURN_TYPE BaseCmdHandler::newResponse(cJSON* pResponse)
{
    char* pResult = new char[MAX_UDP_SIZE+1];
//    char* pJson = cJSON_PrintUnformatted(pResponse);
    char* pJson = cJSON_Print(pResponse);
    strncpy(pResult, pJson, MAX_UDP_SIZE);
    cJSON_Delete(pResponse);
    free(pJson);
    return pResult;
}

bool BaseCmdHandler::isValidStringParam(const char* value, int length)
{
    if (value == NULL){
        return false;
    }
    int len = strlen(value);
    if (len <= 0 || len > length){
        return false;
    }
    return true;
}



