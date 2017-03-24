/**
 * File name: CmdControlLightLevelHandler.cpp
 * Author: Clariones Wang
 *
 * All rights reserved.
 */

#ifndef __CmdControlLightLevelHandler_INCLUDE__
#define __CmdControlLightLevelHandler_INCLUDE__

#include "BaseCmdHandler.h"

class CmdControlLightLevelHandler: public BaseCmdHandler {

public:
    CmdControlLightLevelHandler();
    virtual ~CmdControlLightLevelHandler();

public:
    virtual const char * handle(const char* pCmd, DeltaDoreX2Driver* pDriver);
    virtual const char * getCommandName();
    virtual const char * getUsage();

};

#endif // __CmdControlLightLevelHandler_INCLUDE__
