/**
 * File name: CmdSetNodeNameHandler.cpp
 * Author: Clariones Wang
 *
 * All rights reserved.
 */

#ifndef __CMDSETNODENAMEHANDLER_INCLUDE__
#define __CMDSETNODENAMEHANDLER_INCLUDE__

#include "BaseCmdHandler.h"

class CmdSetNodeNameHandler: public BaseCmdHandler {

public:
    CmdSetNodeNameHandler();
    virtual ~CmdSetNodeNameHandler();

public:
    virtual const char * handle(const char* pCmd, DeltaDoreX2Driver* pDriver);
    virtual const char * getCommandName();
    virtual const char * getUsage();

};

#endif // __CMDSETNODENAMEHANDLER_INCLUDE__