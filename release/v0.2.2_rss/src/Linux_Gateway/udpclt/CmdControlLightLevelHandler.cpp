/**
 * File name: CmdControlLightLevelHandler.cpp
 * Author: Clariones Wang
 *
 * All rights reserved.
 */
#include "CmdControlLightLevelHandler.h"

#define CMD_NAME "controlLightLevel"

CmdControlLightLevelHandler::CmdControlLightLevelHandler(){
}

CmdControlLightLevelHandler::~CmdControlLightLevelHandler(){
}

const char* CmdControlLightLevelHandler::handle(const char* pCmd, DeltaDoreX2Driver* pDriver){
    int network;
    int node;
    int level;

    // first string always the command, so just skip it
    const char* pCurrentParam = pCmd;

    // process paramter: network
    pCurrentParam = getNextParamStartPosition(pCurrentParam);;
    if (pCurrentParam == NULL){
        return newMissingRequiredParametersResponse();
    }
    getParamInt(pCurrentParam, &network);
    if (!isValidNetwork(network)){
        return newWrongIntParamResponse("Invalid network number %d", network);
    }
    // process parameter: node
    pCurrentParam = getNextParamStartPosition(pCurrentParam);
    if (pCurrentParam == NULL){
        return newMissingRequiredParametersResponse();
    }
    getParamInt(pCurrentParam, &node);
    if (!isValidNode(node)){
        return newWrongIntParamResponse("Invalid node number %d", node);
    }

    // process parameter: level
    pCurrentParam = getNextParamStartPosition(pCurrentParam);;
    if (pCurrentParam == NULL){
        return newMissingRequiredParametersResponse();
    }
    getParamInt(pCurrentParam, &level);
    if (!isValidIntParam(level, 0, 255)){
        return newWrongIntParamResponse("invalid level value %d, should be in [0, 100]", level);
    }


    cJSON* pResponse = pDriver->setLightLevel(network, node, level);

    return newResponse(pResponse);
}

const char * CmdControlLightLevelHandler::getCommandName(){
    return CMD_NAME;
}

const char * CmdControlLightLevelHandler::getUsage(){
    return "Control light level\n" \
        "Usage:\n" \
        "    controlLightLevel <network> <node> <level>\n" \
        "Params:\n" \
        "    network: the network number of target device, 0~11\n" \
        "    node: the node number of target device, 0~15\n" \
        "    level: value of the level, 0~100\n";
}

#undef CMD_NAME
