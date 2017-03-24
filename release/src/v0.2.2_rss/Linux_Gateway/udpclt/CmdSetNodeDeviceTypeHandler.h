/**
 * File name: CmdSetNodeNameHandler.cpp
 * Author: Clariones Wang
 *
 * All rights reserved.
 */

#ifndef __CMDSETNODETYPEHANDLER_INCLUDE__
#define __CMDSETNODETYPEHANDLER_INCLUDE__

#include "BaseCmdHandler.h"

class CmdSetNodeDeviceTypeHandler: public BaseCmdHandler {

public:
    CmdSetNodeDeviceTypeHandler();
    virtual ~CmdSetNodeDeviceTypeHandler();

public:
    virtual const char * handle(const char* pCmd, DeltaDoreX2Driver* pDriver);
    virtual const char * getCommandName();
    virtual const char * getUsage();

};

#endif // __CMDSETNODETYPEHANDLER_INCLUDE__
