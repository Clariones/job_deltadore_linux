/*
 * MangementModule.h
 *
 *  Created on: Dec 17, 2016
 *      Author: clari
 */

#ifndef MANGEMENTMODULE_H_
#define MANGEMENTMODULE_H_
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <list>
#include <pthread.h>
#include <semaphore.h>

#include "driver/DeltaDoreX2Driver.h"
#include "udpclt/UdpCmdManager.h"

#include "Helper.h"
#include "DeltaDoreException.h"

using namespace std;
using namespace deltadoreX2d;

namespace cla {

typedef struct {
	MsgActionExecute* pMsg;
	TIME_TS startTimeMs;
	TIME_TS delayTimeMs;
} ActionNotifyMessage;

typedef struct {
	TIME_TS startTimeMs;
	int commandCode;
	int network;
	int node;
	int intParam1;
	int intParam2;
	int intParam3;
	float floatParam;
	string region;
	string name;
} DeviceActionCommand;

class MangementModule {
public:
	MangementModule();
	virtual ~MangementModule();

public:
	void loadConfig(const char* pCfgFileName, const char* pDevCfgFileName) throw (DeltaDoreException);
	void init();
	void start() throw (DeltaDoreException);
	void stop();
	void releaseResource();

	int onObjectUpdateRequest(MsgObjectUpdate& msg, string& errMsg);
	int onObjectDeleteRequest(MsgObjectDelete& msg, string& errMsg);

	void onSetValueAction(MsgActionExecute* pMsg);
	void getAllDeviceInfo(list<MsgObjectUpdate*>& devList);

	ModuleConfig config;

public:
	// should be protected. Now for unit test
	const char* mCfgFileName;
	const char* mDeviceCfgFileName;

	bool started;
	pthread_t pollingThreadId;
	pthread_t messageProcessingThreadId;
	pthread_t commandProcessingThreadId;
	pthread_t webUICommandThreadId;
	sem_t semMessageList;
	sem_t semCommandList;
	sem_t semDeviceOperation;
	sem_t semDeviceList;
	list<ActionNotifyMessage*> actionMessageList;
	list<DeviceActionCommand*> deviceCommandList;
	list<DeviceState*> pDeviceList;

	DeltaDoreX2Driver deltadoreDriver;
	UdpCmdManager udpCommandManager;

public:
	// should be protected. Now for unit test
	// config methods
	void createEmptyCfg();
	void saveDeviceDatas();

	// starting methods
	/** init device with COM command */
	void startDeviceDriver() throw (DeltaDoreException);
	/** Correct loaded config with real devices queried from COM */
	void checkConfigWithDevice() throw (DeltaDoreException);
	/** start to listen at UDP port for web-UI commands */
	void startWebUICommandUdpListeningThread() throw (DeltaDoreException);
	/** start self polling thread for any time-based tasks */
	void startPollingTimerThread() {
		logger(LEVEL_INFO, "  Start polling timer");
		pthread_create(&pollingThreadId, NULL, &funcPollingRoutine, this);
	};
	/** start polling of subscribe message */
	void startMessageProcessingThread() {
		logger(LEVEL_INFO, "  Start MessageProcessingThread");
		pthread_create(&messageProcessingThreadId, NULL, &funcActionMessageProcessingRoutine, this);
	};
	/** start polling device commands */
	void startDeviceCommandProcessingThread(){
		logger(LEVEL_INFO, "  Start DeviceCommandProcessingThread");
		pthread_create(&commandProcessingThreadId, NULL, &funcDeviceCommandProcessingRoutine, this);
	};

	// stopping methods
	#define WAIT_THREAD_STOP(h) if ((h)>0){\
        pthread_join(h,NULL);\
        h=0;\
	}

	void stopPollingTimerThread() {WAIT_THREAD_STOP(pollingThreadId)};
	void stopMessageProcessingThread(){WAIT_THREAD_STOP(messageProcessingThreadId)};
	void stopDeviceCommandProcessingThread(){WAIT_THREAD_STOP(commandProcessingThreadId)};
	void stopWebUICommandUdpListeningThread() {udpCommandManager.stop();};
	void stopDeviceDriver();

	// release resources
	void releaseDeviceResource();
	void releaseWebUIUdpListenerResource();
	void releaseManagerResource();

	// polling thread
	static void* funcPollingRoutine(void* pThis) {
		MangementModule* pMe = (MangementModule*) pThis;
		Helper::sleep_ms(5000);
		while (pMe->started) {
			pMe->pollingRoutine();
			Helper::sleep_ms(1000);
		}
		return pThis;
	}
	void pollingRoutine();

	// action message processing thread
	static void* funcActionMessageProcessingRoutine(void* pThis) {
		MangementModule* pMe = (MangementModule*) pThis;
		while (pMe->started) {
			pMe->actionMessageProcessing();
			Helper::sleep_ms(500);
		}
		return pThis;
	}
	void actionMessageProcessing();

	// device command processing thread
	static void* funcDeviceCommandProcessingRoutine(void* pThis) {
		MangementModule* pMe = (MangementModule*) pThis;
		while (pMe->started) {
			pMe->processDeviceCommands();
			Helper::sleep_ms(100);
		}
		return pThis;
	}
	void processDeviceCommands();

	// device command processing thread
	static void* funcWebUICommandProcessingRoutine(void* pThis) {
		MangementModule* pMe = (MangementModule*) pThis;
		while (pMe->started) {
			// TODO
		}
		return pThis;
	}

	// subsribe message handling
	int addNewDevice(int network, MsgObjectUpdate& msg, string& errMsg);
	DeviceState* findDeviceFromMessage(const string& id);
	void scheduleCommand(MsgActionExecute& msg);
	void pushCommand(TIME_TS delayTimeMs, int commandCode, int network, int node, int pInt1, int pInt2, int pInt3,
			float pFloat1, const char* region, const char* name);
	void pushSetValueCommand(DeviceState* pDev, MsgActionExecute& msg, TIME_TS delayTime);
	inline void setDeviceEnable(DeviceState* pDev, bool enabled) {pDev->enabled = enabled;};
	void publishDeviceDelete(DeviceState* pDevice);
	void publishDeviceState(DeviceState* pDevice);
	void registerDevice(DeviceState* pDevice);
	void executeDeviceCommand(DeviceActionCommand* pCommand, DevicePhysicalState* pResult);

	// device commands
	void getAllDevicePhyStates(list<DevicePhysicalState*>& deviceStateList);
	DevicePhysicalState* findPhyState(list<DevicePhysicalState*>& devPhyList, int network, int node);
	void refreshDevicePhysicalState(bool updateName);
	void updateDeviceState(DeviceState* pDev, DevicePhysicalState* pDevPhy, bool updateName);
	DeviceState* findDeviceList(int network, int node);

	// internal function
	bool addToDeviceList(DeviceState*);
	void sendFakeResponse(DeviceActionCommand* pCommand);
	void sendFakeUpdateMessage(DeviceState* pDevice, const char* value);
	void handleCommandResult(DeviceActionCommand* pCommand, cJSON* pJsonResult);
};
}
#endif /* MANGEMENTMODULE_H_ */
