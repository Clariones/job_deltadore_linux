//
//  testserver.h
//  ModuleSample
//
//  Created by hu_danyuan on 16/12/02.
//  Copyright © 2016年 杭州速凡网络科技有限公司. All rights reserved.
//
//  订阅服务

#ifndef __DELTA_DORE_SERVER_H__
#define __DELTA_DORE_SERVER_H__

#include <string>
#include "SF_ModBridge/BaseModule.h"
#include "MangementModule.h"
#include "DtdMngService.h"
#include "DtdMngSubscribe.h"
#include "utils/fileUtils.h"

namespace sf {
	class BaseModule;
}
#ifdef __cplusplus
extern "C"{
#endif

sf::BaseModule *DeltaDoreDriver_load();
int DeltaDoreDriver_init(unsigned char flag);
int DeltaDoreDriver_start();
void DeltaDoreDriver_stop();
void DeltaDoreDriver_deinit();

using namespace cla;
extern MangementModule* pManager;
extern DtdMngService* pService;
extern DtdMngSubscribe* pSubscribe;

#ifdef __cplusplus
}
#endif

#endif /* __DELTA_DORE_SERVER_H__ */
