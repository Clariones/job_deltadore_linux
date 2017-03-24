/**
 * File name: CmdSetNodeNameHandler.cpp
 * Author: Clariones Wang
 *
 * All rights reserved.
 */
#include "CmdSetNodeNameHandler.h"

#define CMD_NAME "setNodeName"

CmdSetNodeNameHandler::CmdSetNodeNameHandler(){
}

CmdSetNodeNameHandler::~CmdSetNodeNameHandler(){
}

const char* CmdSetNodeNameHandler::handle(const char* pCmd, DeltaDoreX2Driver* pDriver){
    int network;
    int node;
    char* name;
    char* region;
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
    region = new char[strlen(pCurrentParam) +1];
    getParamString(pCurrentParam, region);
    if (!isValidStringParam(region, MAX_NAME_LENGTH)){
        delete region;
        return newWrongStringParamResponse("invalid region value, should be string with length [1, 30]", region);
    }

    // process parameter: name
    pCurrentParam = getNextParamStartPosition(pCurrentParam);;
    if (pCurrentParam == NULL){
        return newMissingRequiredParametersResponse();
    }
    name = new char[strlen(pCurrentParam) +1];
    getParamString(pCurrentParam, name);
    if (!isValidStringParam(name, MAX_NAME_LENGTH)){
        delete region;
        delete name;
        return newWrongStringParamResponse("invalid name value, should be string with length [1, 30]", name);
    }

    cJSON* pResponse = pDriver->setNodeName(network, node, region, name);

    delete region;
    delete name;
    return newResponse(pResponse);
}

const char * CmdSetNodeNameHandler::getCommandName(){
    return CMD_NAME;
}

const char * CmdSetNodeNameHandler::getUsage(){
    return "Assign a friendly name to a node\n" \
        "Usage:\n" \
        "    setNodeName <network> <node> <region> <name>\n" \
        "Params:\n" \
        "    network: the network number of target device, 0~11\n" \
        "    node: the node number of target device, 0~15\n" \
        "    region: region of the node\n" \
        "    name: name of the node";
}

#undef CMD_NAME
