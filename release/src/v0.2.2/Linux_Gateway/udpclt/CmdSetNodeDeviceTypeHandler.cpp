/**
 * File name: CmdSetNodeDeviceTypeHandler.cpp
 * Author: Clariones Wang
 *
 * All rights reserved.
 */
#include "CmdSetNodeDeviceTypeHandler.h"

#define CMD_NAME "setNodeDeviceType"

CmdSetNodeDeviceTypeHandler::CmdSetNodeDeviceTypeHandler(){
}

CmdSetNodeDeviceTypeHandler::~CmdSetNodeDeviceTypeHandler(){
}

const char* CmdSetNodeDeviceTypeHandler::handle(const char* pCmd, DeltaDoreX2Driver* pDriver){
    int network;
    int node;
    char* deviceType;
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

    // process parameter: region
    pCurrentParam = getNextParamStartPosition(pCurrentParam);;
    if (pCurrentParam == NULL){
        return newMissingRequiredParametersResponse();
    }
    deviceType = new char[strlen(pCurrentParam) +1];
    getParamString(pCurrentParam, deviceType);
    if (strcmp("light", deviceType) != 0 && strcmp("dimmer", deviceType) != 0 && strcmp("rollerShutter", deviceType) != 0){
        delete deviceType;
        return newWrongStringParamResponse("invalid device type, should be light, dimmer or rollerShutter", deviceType);
    }

    cJSON* pResponse = pDriver->setNodeDeviceType(network, node, deviceType);

    delete deviceType;
    return newResponse(pResponse);
}

const char * CmdSetNodeDeviceTypeHandler::getCommandName(){
    return CMD_NAME;
}

const char * CmdSetNodeDeviceTypeHandler::getUsage(){
    return "Set a device type to a node\n" \
        "Usage:\n" \
        "    setNodeName <network> <node> <region> <name>\n" \
        "Params:\n" \
        "    network: the network number of target device, 0~11\n" \
        "    node: the node number of target device, 0~15\n" \
        "    device type: device type of the node\n" \
        "          can be light, dimmer or rollerShutter";
}

#undef CMD_NAME
