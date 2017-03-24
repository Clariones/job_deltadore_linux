//
//  testserver.cpp
//  ModuleSample
//
//  Created by hu_danyuan on 16/12/02.
//  Copyright © 2016年 杭州速凡网络科技有限公司. All rights reserved.
//

#include "DeltaDoreServer.h"
#include <stdio.h>
#include <iostream>

#include "SF_ModBridge/BaseModule.h"
#include "SF_ModBridge/ExtServicesManager.h"
#include "SF_ModBridge/SubServicesManager.h"
#include "MangementModule.h"
#include "DeltaDoreException.h"

using namespace cla;

MangementModule* pManager = NULL;
DtdMngService* pService = NULL;
DtdMngSubscribe* pSubscribe = NULL;

#ifdef _WIN32
	#define CFG_FILE "testinput/dtdconfig.json"
	#define DATA_FILE "testinput/dtddata.json"
#else
	#define CFG_FILE "/etc/luoping/save/dtdconfig.json"
	#define DATA_FILE "/etc/luoping/save/dtddata.json"
#endif

using namespace sf;
namespace deltadore {
static sf::BaseModule module_info;
}
sf::BaseModule *DeltaDoreDriver_load() {
	deltadore::module_info.name = DEFAULT_MODULE_ID;
	deltadore::module_info.version = "1.0.0";
	deltadore::module_info.secret = "04064c009bda2a6ba640d55549642dcb";
	deltadore::module_info.init = DeltaDoreDriver_init;
	deltadore::module_info.deinit = DeltaDoreDriver_deinit;
	deltadore::module_info.start = DeltaDoreDriver_start;
	deltadore::module_info.stop = DeltaDoreDriver_stop;
	logger(LEVEL_INFO, "load module %s:", deltadore::module_info.name.c_str());
	return &deltadore::module_info;
}
int DeltaDoreDriver_init(unsigned char flag) {
	//std::cout<<"\033[31mhdy_init\033[0m"<<std::endl;
	logger(LEVEL_INFO, "init module %s:", deltadore::module_info.name.c_str());
	pManager = new MangementModule();
	try {
		pManager->loadConfig(CFG_FILE, DATA_FILE);
		pManager->init();

		pService = new DtdMngService();
		pService->setManager(pManager);

		pSubscribe = new DtdMngSubscribe();
		pSubscribe->setManager(pManager);

	} catch (DeltaDoreException e) {
	    logger(LEVEL_ERROR, "DeltaDoreDriver_start() failed with errorcode %d", e.getCode());
		return -1;
	}
	return 0;
}
int DeltaDoreDriver_start() {
	logger(LEVEL_INFO, "start module %s:", deltadore::module_info.name.c_str());
	try {
		// start device driver manager
		pManager->start();
		// subscribe
		SubServicesManager::instance()->subscribe(kSubscribeServiceObjectState, pSubscribe);
		SubServicesManager::instance()->subscribe(kSubscribeServiceActionExecute, pSubscribe);
		// register service
		ExtServicesManager::instance()->registerService(DEFAULT_MODULE_ID, pService);

		pSubscribe->start();
		pService->start();
	} catch (DeltaDoreException e) {
	    logger(LEVEL_ERROR, "DeltaDoreDriver_start() failed with errorcode %d", e.getCode());
		return -1;
	}
	return 0;
}
void DeltaDoreDriver_stop() {
	ExtServicesManager::instance()->unregisterService(pService);
	SubServicesManager::instance()->unsubscribe(pSubscribe);
	pService->stop();
	pSubscribe->stop();
	logger(LEVEL_INFO, "stopping DeltaDoreDriver module");
	pManager->stop();
}
void DeltaDoreDriver_deinit() {
	logger(LEVEL_INFO, "deinit DeltaDoreDriver");
	pManager->releaseResource();
	delete pManager;
	pManager = NULL;

	delete pSubscribe;
	pSubscribe = NULL;
	delete pService;
	pService = NULL;
}
