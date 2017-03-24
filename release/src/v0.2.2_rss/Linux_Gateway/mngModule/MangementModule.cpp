/*
 * MangementModule.cpp
 *
 *  Created on: Dec 17, 2016
 *      Author: clari
 */

#include "SF_ModBridge/ExtServicesManager.h"
#include "SF_ModBridge/SubServicesManager.h"
#include "MangementModule.h"
#include "DeltaDoreException.h"

#define REAL_STATE_RESPONSE true

#define version "V0.2.2_rss"

#define LOCK_DEVICE_LIST //sem_wait(&semDeviceList);

#define UNLOCK_DEVICE_LIST //sem_post(&semDeviceList);

#define UNCONDITON_REPORT_TIME (60*1000)

extern TIME_TS TEST_DEVICE_TS;

using namespace std;
namespace cla {
void releaseMsgActionExecute(MsgActionExecute* pMsg) {
	delete pMsg;
}

DeviceState* createNewDeviceState() {
    DeviceState* pDev = new DeviceState();
    pDev->deviceType = DEVICE_UNKNOWN;
    pDev->onLine = false;
    pDev->mustReportStateTs = 0;
    return pDev;
}

MangementModule::MangementModule() {
    pollingThreadId = 0;
	messageProcessingThreadId = 0;
	commandProcessingThreadId = 0;
	webUICommandThreadId = 0;
}

MangementModule::~MangementModule() {
	if (started) {
		stop();
		releaseResource();
	}
}
/**
 * Starting procedure, please see DeltaDoreDriver_xxxx.
 * For this module, it is:
 * loadConfig() -> start() ....... -> stop() -> releaseResource()
 *
 */
void MangementModule::loadConfig(const char* pCfgFileName, const char* pDevCfgFileName) throw (DeltaDoreException) {
	mCfgFileName = pCfgFileName;
	mDeviceCfgFileName = pDevCfgFileName;

	createEmptyCfg();
	if (!fileExisted(mCfgFileName)) {
		logger(LEVEL_INFO, "Cannot found config file, so create empty one");
	} else {
		char* cfgConent = readFileAsString(mCfgFileName);
		int pos = Helper::lastIndexOf(cfgConent, "}");
		if (pos < -1) {
			logger(LEVEL_ERROR, "Invalid JSON string");
			EXIT_WITH_ERROR(ERR_CODE_LOADCONFIG);
		}
		cfgConent[pos + 1] = 0;
		logger(LEVEL_INFO, "got config JSON:\n%s\n", cfgConent);
		Helper::parseConfigFromJson(cfgConent, config);
	}

	if (!fileExisted(mDeviceCfgFileName)) {
		logger(LEVEL_INFO, "Cannot found device data file.");
	} else {
		char* cfgConent = readFileAsString(mDeviceCfgFileName);
		int pos = Helper::lastIndexOf(cfgConent, "]");
		if (pos < -1) {
			logger(LEVEL_ERROR, "Invalid JSON string");
			EXIT_WITH_ERROR(ERR_CODE_LOADCONFIG);
		}
		cfgConent[pos + 1] = 0;
		logger(LEVEL_INFO, "got devices data JSON:\n%s\n", cfgConent);
		Helper::parseDeviceConfigFromJson(cfgConent, pDeviceList);
		delete cfgConent;
	}
	list<DeviceState*>::iterator it = pDeviceList.begin();
	for(;it != pDeviceList.end();it++){
		DeviceState* pDev = *it;
		if (pDev->type == CONST_TYPE_DIMMER){
			pDev->value="0";
			pDev->level = 0;
		}else if (pDev->type == CONST_TYPE_SHADE){
			pDev->value="0";
			pDev->position = 0;
		}else{
			pDev->value = "off";
			pDev->level = 0;
		}
	}
}

void MangementModule::createEmptyCfg() {
	config.moduleId = DEFAULT_MODULE_ID;
	config.webCommandPort = 1800;
	pDeviceList.clear();
	config.deviceName = DEFAULT_DEVICE_NAME;
	config.manufacturer = DEFAULT_MANUFACTURE;
}

void MangementModule::saveDeviceDatas() {
	cJSON* pData = Helper::toJsonDeviceDatas(pDeviceList);
	char* pJsonStr = cJSON_Print(pData);
	writeStringToFile(mDeviceCfgFileName, pJsonStr);
	free(pJsonStr);
	cJSON_Delete(pData);
}
/*************************** 启动过程相关函数************************************/
void MangementModule::start() throw (DeltaDoreException) {
    this->started = true;
    logger(LEVEL_INFO, "start deltadore management %s", version);
	startDeviceDriver();
	checkConfigWithDevice();
	startWebUICommandUdpListeningThread();
	startPollingTimerThread();
	startDeviceCommandProcessingThread();
	startMessageProcessingThread();
}

void MangementModule::startDeviceDriver() throw (DeltaDoreException) {
	logger(LEVEL_INFO, "  Start device driver");
	// TODO
	deltadoreDriver.init(config.deviceName.c_str());
	if (!deltadoreDriver.initSuccess()){
        logger(LEVEL_ERROR, "DeltaDore Driver init failed.\n");
        throw DeltaDoreException(ERR_CODE_DRIVER_INIT_FAIL);
    }
}
void MangementModule::checkConfigWithDevice() throw (DeltaDoreException) {
	refreshDevicePhysicalState(false);
	// TODO debug
    printf("\n\n============initial query finished =============\n\n");
	cJSON* pResult = NULL;
	// when start, not send any message. Just remove any "DELETED" and update the name
	//LOCK_DEVICE_LIST
	list<DeviceState*>::iterator it = pDeviceList.begin();
	for (; it != pDeviceList.end();) {
		DeviceState* pDev = *it;
		if (pDev->refreshResult == REFRESH_TYPE_DELETED) {
			it = pDeviceList.erase(it);
			delete pDev;
		} else {
		    printf("Device PHY type is (%d.%d)=%d,%s\n", pDev->network, pDev->node, pDev->deviceType, pDev->type.c_str());
			if (pDev->type == CONST_TYPE_LIGHT){
                deltadoreDriver.setNodeDeviceType(pDev->network, pDev->node, "light");
			}else if (pDev->type == CONST_TYPE_DIMMER){
                deltadoreDriver.setNodeDeviceType(pDev->network, pDev->node, "dimmer");
			}else if (pDev->type == CONST_TYPE_SHADE){
				deltadoreDriver.setNodeDeviceType(pDev->network, pDev->node, "rollerShutter");
			}else{
				// 如果启动时没办法确定设备类型，那就什么都不做
				logger(LEVEL_INFO, "device %d.%d type cannot be decided when start up.", pDev->network, pDev->node);
				pDev->onLine = false;
			}
		    pResult = deltadoreDriver.setNodeName(pDev->network, pDev->node, pDev->region.c_str(), pDev->name.c_str());
		    pDev->refreshResult = REFRESH_TYPE_SET_BY_CMD;
		    if (pResult != NULL){
                cJSON_Delete(pResult);
		    }
			it++;
		}
	}
	//UNLOCK_DEVICE_LIST
}
void MangementModule::startWebUICommandUdpListeningThread() throw (DeltaDoreException) {
	logger(LEVEL_INFO, "  Start WEB-UI command listener at port %d", config.webCommandPort);
	udpCommandManager.setPort(config.webCommandPort);
	udpCommandManager.setDriver(&deltadoreDriver);
	webUICommandThreadId = udpCommandManager.start();
}

/*************************** 轮询任务 ***************************************/
/**
 * 轮询任务。 主要作用是定时检测设备状态， 发现有状态需要刷新时，调用刷新命令来刷新设备的状态。
 * 超时时间默认为1分钟。 如果有操作命令，则会立即设置 nextRefresh的时间值，所以理论延迟是一个轮询周期，即1s。
 */
void MangementModule::pollingRoutine() {
	//logger(LEVEL_DEBUG, "pollingRoutine ran once");
	list<DevicePhysicalState*> devPhyStates;
	getAllDevicePhyStates(devPhyStates);

	DevicePhysicalState tempResult;

	// check all device refresh time.
	while (devPhyStates.size() > 0) {
		DevicePhysicalState* pDevPhy = devPhyStates.front();
		devPhyStates.pop_front();

		TIME_TS curTs = Helper::getCurrentSystemMS();
		if (Helper::timeIsUp(curTs, pDevPhy->nextRefreshTs)) {
            if (pDevPhy->deviceType == DEVICE_LIGHT) {
				DeviceActionCommand cmd;
				cmd.commandCode = CMD_GET_LIGHT_STATE;
				cmd.network = pDevPhy->network;
				cmd.node = pDevPhy->node;
				executeDeviceCommand(&cmd, &tempResult);
			} else if (pDevPhy->deviceType == DEVICE_ROLLER_SHUTTER) {
				DeviceActionCommand cmd;
				cmd.commandCode = CMD_GET_ROLLERSHUTTER_STATE;
				cmd.network = pDevPhy->network;
				cmd.node = pDevPhy->node;
				executeDeviceCommand(&cmd, &tempResult);
			} else if (pDevPhy->deviceType == DEVICE_TEMPERATURE_SENSOR) {
				DeviceActionCommand cmd;
				cmd.commandCode = CMD_GET_TEMPERATURE_SENSOR_STATE;
				cmd.network = pDevPhy->network;
				cmd.node = pDevPhy->node;
				executeDeviceCommand(&cmd, &tempResult);
			} else {
				DeviceActionCommand cmd;
				cmd.commandCode = CMD_REFRESH_DEVICE_STATE;
				cmd.network = pDevPhy->network;
				cmd.node = pDevPhy->node;
				logger(LEVEL_DEBUG, "==========What are you?==============");
				executeDeviceCommand(&cmd, &tempResult);
			}
		}
		delete pDevPhy;
	}

	// after read-refresh, re-load all state
	refreshDevicePhysicalState(true);
	LOCK_DEVICE_LIST
	list<DeviceState*>::iterator it = pDeviceList.begin();
	for (; it != pDeviceList.end();) {
		DeviceState* pDev = *it;

		if (pDev->refreshResult == REFRESH_TYPE_DELETED) {
			it = pDeviceList.erase(it);
			publishDeviceDelete(pDev);
			delete pDev;
			continue;
		}
		it++;

		if (pDev->refreshResult == REFRESH_TYPE_SET_BY_CMD) {
            logger(LEVEL_INFO, "pushlish device state because changed by command or just init");
            registerDevice(pDev);
			publishDeviceState(pDev);
		} else if (pDev->refreshResult == REFRESH_TYPE_UPDATED) {
		    logger(LEVEL_INFO, "pushlish device state because value changed");
			publishDeviceState(pDev);
		} else if (pDev->refreshResult == REFRESH_TYPE_NEW) {
		    logger(LEVEL_INFO, "pushlish device state because new device found");
			registerDevice(pDev);
			publishDeviceState(pDev);
		} else if (pDev->refreshResult == REFRESH_TYPE_OFFLINE){
			logger(LEVEL_INFO, "pushlish device state because offline");
			registerDevice(pDev);
			publishDeviceState(pDev);
		} else if (pDev->refreshResult == REFRESH_TYPE_ONLINE){
			logger(LEVEL_INFO, "pushlish device state because online");
			registerDevice(pDev);
			publishDeviceState(pDev);
		}
		pDev->refreshResult = REFRESH_TYPE_NOCHANGE;
	}
	UNLOCK_DEVICE_LIST
}

/*************************** 通知消息set-value的处理函数***************************************/
/**
 * 收到的任务通知消息，会被处理。 最复杂的是 set-value命令，会发送到actionMessageList里，在这里再真正处理。
 * 轮询间隔是100ms。
 */
void MangementModule::actionMessageProcessing() {
	//logger(LEVEL_DEBUG, "actionMessageProcessing ran once");
	list<MsgActionExecute> commandsNeedSchedule;

	sem_wait(&semMessageList);
	// 以下保护区中，只挑出需要处理的命令，放入队列，但是不直接处理，出了保护区才处理命令
	list<ActionNotifyMessage*>::iterator msgProcIt;
	for (msgProcIt = actionMessageList.begin(); msgProcIt != actionMessageList.end();) {
		TIME_TS curTs = Helper::getCurrentSystemMS();
		ActionNotifyMessage* pNotify = *msgProcIt;
		TIME_TS wantedTs = pNotify->startTimeMs + pNotify->delayTimeMs;
		if (!Helper::timeIsUp(curTs, wantedTs)) {
			msgProcIt++;
			continue;
		}
		MsgActionExecute& msg = *(pNotify->pMsg);
		switch (msg.action_type) {
		case ACTION_TYPE_Repeat: {
			// repeat 类型的操作，每次减计数，并重置启动时间
			msg.value_count--;
			pNotify->startTimeMs += msg.value_period * 1000;
			// 为每个单独的命令，下发计划任务
			list<MsgActionExecute*>::iterator it;
			for (it = msg.repeatActions.begin(); it != msg.repeatActions.end(); it++) {
				MsgActionExecute* pSingleMsg = *it;
				try {
					//scheduleCommand(*pSingleMsg);
					commandsNeedSchedule.push_back(*pSingleMsg);
				} catch (DeltaDoreException e) {
					logger(LEVEL_ERROR, "Skip action because error code(%d)", e.getCode());
				}
			}
			// 每次处理完了以后，检查是不是计数到头了
			if (msg.value_count <= 0) {
				// 如果计数到头了，就需要释放资源了
				while (msg.repeatActions.size() > 0) {
					MsgActionExecute* pAction = msg.repeatActions.back();
					msg.repeatActions.pop_back();
					//delete pAction;
				}
				delete pNotify->pMsg;
				msgProcIt = actionMessageList.erase(msgProcIt);
			} else {
				msgProcIt++;
			}
			break;
		}
		case ACTION_TYPE_cycleOnOff: {
			msg.value_count--;
			pNotify->startTimeMs += (msg.value_delay_on + msg.value_delay_off) * 1000;
			try {
				//scheduleCommand(msg);
				commandsNeedSchedule.push_back(msg);
			} catch (DeltaDoreException e) {
				logger(LEVEL_ERROR, "Skip action because error code(%d)", e.getCode());
				delete pNotify->pMsg;
				msgProcIt = actionMessageList.erase(msgProcIt);
				break;
			}
			if (msg.value_count <= 0) {
				//delete pNotify->pMsg;
				msgProcIt = actionMessageList.erase(msgProcIt);
			} else {
				msgProcIt++;
			}
			break;
		}
		default: {
			try {
				//scheduleCommand(msg);
				commandsNeedSchedule.push_back(msg);
			} catch (DeltaDoreException e) {
				logger(LEVEL_ERROR, "Skip action because error code(%d)", e.getCode());
			}
			//delete pNotify->pMsg;
			msgProcIt = actionMessageList.erase(msgProcIt);
			break;
		}
		}
	}
	sem_post(&semMessageList);

	while(commandsNeedSchedule.size() > 0){
		MsgActionExecute cmd = commandsNeedSchedule.back();
		scheduleCommand(cmd);
		commandsNeedSchedule.pop_back();
		delete &cmd;
	}
}
/**
 * 从actionMessageList里得到的命令，有一些是有时间控制的，所以会构造一个带时间戳的设备命令，放入deviceCommandList中去
 */
void MangementModule::scheduleCommand(MsgActionExecute& msg) {
	DeviceState* pDev = NULL;

	// verify if the command has valid device ID
	switch (msg.action_type) {
	case ACTION_TYPE_SetValue:
	case ACTION_TYPE_toggleValue:
	case ACTION_TYPE_refreshState:
	case ACTION_TYPE_cycleOnOff:
	case ACTION_TYPE_delayOff: {
		pDev = findDeviceFromMessage(msg.id);
		if (pDev == NULL) {
			logger(LEVEL_DEBUG, "Cannot find device %s", pDev->id.c_str());
			throw DeltaDoreException(ERR_CODE_CMD_INVALID_ID);
		}
		if (!pDev->enabled) {
			// disabled device do not accept any set-value command
			logger(LEVEL_DEBUG, "device %d,%d was disable", pDev->network, pDev->node);
			return;
		}
		break;
	}

	case ACTION_TYPE_setEnable: {
		pDev = findDeviceFromMessage(msg.id);
		if (pDev == NULL) {
			throw DeltaDoreException(ERR_CODE_CMD_INVALID_ID);
		}
		break;
	}
	}

	switch (msg.action_type) {
	case ACTION_TYPE_SetValue: {
		pushSetValueCommand(pDev, msg, 0);
		break;
	}
	case ACTION_TYPE_SetAllValue: {
		list<DeviceState*>::iterator it = pDeviceList.begin();
		for (; it != pDeviceList.end(); it++) {
			DeviceState* pDev = *it;
			if (msg.clusterid != pDev->cluster){
				continue;
			}
			pushSetValueCommand(pDev, msg, 0);
		}
		break;
	}
	case ACTION_TYPE_toggleValue: {
		if (pDev->deviceType == DEVICE_LIGHT) {
			pushCommand(0, CMD_SET_LIGHT_TOGGLE, pDev->network, pDev->node, 0, 0, 0, 0, NULL, NULL);
		}
		break;
	}
	case ACTION_TYPE_refreshState: {
		pDev->refreshResult = REFRESH_TYPE_SET_BY_CMD;
		switch (pDev->deviceType) {
		case DEVICE_LIGHT:
			pushCommand(0, CMD_GET_LIGHT_STATE, pDev->network, pDev->node, 0, 0, 0, 0, NULL, NULL);
			break;
		case DEVICE_ROLLER_SHUTTER:
			pushCommand(0, CMD_GET_ROLLERSHUTTER_STATE, pDev->network, pDev->node, 0, 0, 0, 0, NULL, NULL);
			break;
		case DEVICE_TEMPERATURE_SENSOR:
			pushCommand(0, CMD_GET_TEMPERATURE_SENSOR_STATE, pDev->network, pDev->node, 0, 0, 0, 0, NULL, NULL);
			break;
		}
		break;
	}
	case ACTION_TYPE_cycleOnOff: {
		// repeat was handled by upper layer (actionMessageProcessing), here just push command
		if (pDev->deviceType == DEVICE_LIGHT) {
			pushCommand(msg.value_delay_on * 1000, CMD_SET_LIGHT_ON, pDev->network, pDev->node, 0, 0, 0, 0, NULL, NULL);
			pushCommand((msg.value_delay_on + msg.value_delay_off) * 1000, CMD_SET_LIGHT_OFF, pDev->network, pDev->node,
					0, 0, 0, 0, NULL, NULL);
		} else if (pDev->deviceType == DEVICE_ROLLER_SHUTTER) {
			pushCommand(msg.value_delay_on * 1000, CMD_SET_ROLLERSHUTTER_OPEN, pDev->network, pDev->node, 0, 0, 0, 0,
					NULL, NULL);
			pushCommand((msg.value_delay_on + msg.value_delay_off) * 1000, CMD_SET_ROLLERSHUTTER_CLOSE, pDev->network,
					pDev->node, 0, 0, 0, 0, NULL, NULL);
		}
		break;
	}
	case ACTION_TYPE_delayOff: {
		if (pDev->deviceType == DEVICE_LIGHT) {
			pushCommand(msg.value_delaytime * 1000, CMD_SET_LIGHT_OFF, pDev->network, pDev->node, 0, 0, 0, 0, NULL,
					NULL);
		} else if (pDev->deviceType == DEVICE_ROLLER_SHUTTER) {
			pushCommand(msg.value_delaytime * 1000, CMD_SET_ROLLERSHUTTER_CLOSE, pDev->network, pDev->node, 0, 0, 0, 0,
					NULL, NULL);
		}
		break;
	}
	case ACTION_TYPE_setEnable: {
		setDeviceEnable(pDev, msg.value_enable);
		break;
	}
	case ACTION_TYPE_queryObjects: {
		LOCK_DEVICE_LIST
		list<DeviceState*>::iterator it = pDeviceList.begin();
		for (; it != pDeviceList.end(); it++) {
            registerDevice(*it);
			publishDeviceState(*it);
		}
		UNLOCK_DEVICE_LIST
		break;
	}
	}
}
void MangementModule::pushSetValueCommand(DeviceState* pDev, MsgActionExecute& msg, TIME_TS delayTime) {
	switch (pDev->deviceType) {
	case DEVICE_LIGHT: {
		if (msg.value_level < 0) {
			if (msg.value_onOff) {
				pushCommand(delayTime, CMD_SET_LIGHT_ON, pDev->network, pDev->node, 0, 0, 0, 0, NULL, NULL);
			} else {
				pushCommand(delayTime, CMD_SET_LIGHT_OFF, pDev->network, pDev->node, 0, 0, 0, 0, NULL, NULL);
			}
		} else {
			pushCommand(delayTime, CMD_SET_LIGHT_LEVEL, pDev->network, pDev->node, 0x00FF &(msg.value_level*100/255), 0, 0, 0, NULL,
					NULL);
		}
		break;
	}
	case DEVICE_ROLLER_SHUTTER: {
	    // TODO debug
	    // logger(LEVEL_DEBUG, "Message value is %s, value_level is %d", msg.value.c_str(), msg.value_level);
		if (msg.value == "close" || msg.value == "off" || msg.value_level == 0) {
			pushCommand(delayTime, CMD_SET_ROLLERSHUTTER_CLOSE, pDev->network, pDev->node, 0, 0, 0, 0, NULL, NULL);
		} else if (msg.value == "open" || msg.value == "on" || msg.value_level > 0) {
			pushCommand(delayTime, CMD_SET_ROLLERSHUTTER_OPEN, pDev->network, pDev->node, 0, 0, 0, 0, NULL, NULL);
		} else if (msg.value == "stop") {
			pushCommand(delayTime, CMD_SET_ROLLERSHUTTER_STOP, pDev->network, pDev->node, 0, 0, 0, 0, NULL, NULL);
		} else {
			logger(LEVEL_ERROR, "Unsupported action %s for roller-shutter %s", msg.value.c_str(), pDev->id.c_str());
		}
		break;
	}
	default:
		logger(LEVEL_ERROR, "Unsupported device type for device id %s", pDev->id.c_str());
		break;
	}
}
void MangementModule::pushCommand(TIME_TS delayTimeMs, int commandCode, int network, int node, int pInt1, int pInt2,
		int pInt3, float pFloat1, const char* region, const char* name) {
	DeviceActionCommand* pCommand = new DeviceActionCommand();
	pCommand->commandCode = commandCode;
	pCommand->network = network;
	pCommand->node = node;
	pCommand->intParam1 = pInt1;
	pCommand->intParam2 = pInt2;
	pCommand->intParam3 = pInt3;
	pCommand->floatParam = pFloat1;
	if (region != NULL) {
		pCommand->region = region;
	}
	if (name != NULL) {
		pCommand->name = name;
	}
	pCommand->startTimeMs = Helper::getCurrentSystemMS() + delayTimeMs;
	sem_wait(&semCommandList);
	deviceCommandList.push_back(pCommand);
	if (!REAL_STATE_RESPONSE) {
		sendFakeResponse(pCommand);
	}
	sem_post(&semCommandList);

}
void MangementModule::sendFakeResponse(DeviceActionCommand* pCommand){
	char buff[20];
	DeviceState* pDevice = findDeviceList(pCommand->network, pCommand->node);
	if (pDevice == NULL){
		logger(LEVEL_DEBUG, "device %d.%d not found. No auto response.\n", pCommand->network, pCommand->node);
		return;
	}
	// if (pDevice->onLine == false){
	// 	logger(LEVEL_DEBUG, "device %d.%d is offline. No auto response.\n", pCommand->network, pCommand->node);
	// 	return;
	// }

	switch (pCommand->commandCode) {
	case CMD_SET_LIGHT_ON: {
		logger(LEVEL_DEBUG, "response switchOnLight(%d,%d)", pCommand->network, pCommand->node);
		if (pDevice->type == CONST_TYPE_LIGHT){
			pDevice->value = "on";
			pDevice->level = 255;
			sendFakeUpdateMessage(pDevice, "on");
		}else{
			pDevice->value = "255";
			pDevice->level = 255;
			sendFakeUpdateMessage(pDevice, "255");
		}
		break;
	}
	case CMD_SET_LIGHT_OFF: {
		logger(LEVEL_DEBUG, "response switchOffLight(%d,%d)", pCommand->network, pCommand->node);
		if (pDevice->type == CONST_TYPE_LIGHT){
			pDevice->value = "off";
			pDevice->level = 0;
			sendFakeUpdateMessage(pDevice, "off");
		}else{
			pDevice->value = "0";
			pDevice->level = 0;
			sendFakeUpdateMessage(pDevice, "0");
		}
		break;
	}
	case CMD_SET_LIGHT_LEVEL: {
		logger(LEVEL_DEBUG, "response setLightLevel(%d,%d,%d)", pCommand->network, pCommand->node, pCommand->intParam1);
		int level4Msg = pCommand->intParam1 * 255 / 100;
		sprintf(buff, "%d", level4Msg);
		pDevice->value = buff;
		pDevice->level = level4Msg;
		sendFakeUpdateMessage(pDevice, buff);
		break;
	}
	case CMD_SET_LIGHT_TOGGLE: {
		logger(LEVEL_DEBUG, "response toggleLight(%d,%d)", pCommand->network, pCommand->node);
		if (pDevice->level > 0){
			if (pDevice->type == CONST_TYPE_LIGHT){
				pDevice->value = "off";
				pDevice->level = 0;
				sendFakeUpdateMessage(pDevice, "off");
			}else{
				pDevice->value = "0";
				pDevice->level = 0;
				sendFakeUpdateMessage(pDevice, "0");
			}
		}else{
			if (pDevice->type == CONST_TYPE_LIGHT){
				pDevice->value = "on";
				pDevice->level = 255;
				sendFakeUpdateMessage(pDevice, "on");
			}else{
				pDevice->value = "255";
				pDevice->level = 255;
				sendFakeUpdateMessage(pDevice, "255");
			}
		}
		break;
	}
	case CMD_SET_ROLLERSHUTTER_OPEN: {
		logger(LEVEL_DEBUG, "response openRollerShutter(%d,%d)", pCommand->network, pCommand->node);
		pDevice->value = "255";
		pDevice->position = 100;
		sendFakeUpdateMessage(pDevice, "255");
		break;
	}
	case CMD_SET_ROLLERSHUTTER_CLOSE: {
		pDevice->value = "0";
		pDevice->position = 0;
		logger(LEVEL_DEBUG, "response closeRollerShutter(%d,%d)", pCommand->network, pCommand->node);
		sendFakeUpdateMessage(pDevice, "0");
		break;
	}
	case CMD_SET_ROLLERSHUTTER_STOP: {
		logger(LEVEL_DEBUG, "response stopRollerShutter(%d,%d)", pCommand->network, pCommand->node);
		pDevice->value = "127";
		pDevice->position = 110;
		sendFakeUpdateMessage(pDevice, "127");
		break;
	}
	default: break;
	}
}
void MangementModule::processDeviceCommands() {
	//logger(LEVEL_DEBUG, "processDeviceCommands ran once");
	list<DeviceActionCommand*> readyCommands;
	sem_wait(&semCommandList);
	TIME_TS curTs = Helper::getCurrentSystemMS();
	list<DeviceActionCommand*>::iterator it;
	for (it = deviceCommandList.begin(); it != deviceCommandList.end();) {
		DeviceActionCommand* pCommand = *it;
		if (Helper::timeIsUp(curTs, pCommand->startTimeMs)) {
			readyCommands.push_back(pCommand);
			it = deviceCommandList.erase(it);
		} else {
			it++;
		}
	}
	sem_post(&semCommandList);

	DevicePhysicalState result;
	while (readyCommands.size() > 0) {
		DeviceActionCommand* pCommand = readyCommands.front();
		executeDeviceCommand(pCommand, &result);
		delete pCommand;
		readyCommands.pop_front();
	}
	readyCommands.clear();
}



DevicePhysicalState* MangementModule::findPhyState(list<DevicePhysicalState*>& devPhyList, int network, int node) {
	list<DevicePhysicalState*>::iterator it = devPhyList.begin();
	for (; it != devPhyList.end(); it++) {
		DevicePhysicalState* pNow = *it;
		if (pNow->network == network && pNow->node == node) {
			return pNow;
		}
	}
	return NULL;
}

void MangementModule::publishDeviceDelete(DeviceState* pDevice) {
	string key = "SOFI_HCC";
	string body;
	string response;
	Helper::toJsonReqObjectUnReg(pDevice->id, body);
	sf::ExtServicesManager::instance()->request(key, config.moduleId, body, response);
	logger(LEVEL_INFO, "Request Object.UnReg result is %s\n", response.c_str());
}
void MangementModule::registerDevice(DeviceState* pDevice) {
	string key = "SOFI_HCC";
	string body;
	string response;
	Helper::toJsonRequestObjectReg(pDevice, config.moduleId, body);
	sf::ExtServicesManager::instance()->request(key, config.moduleId, body, response);
	logger(LEVEL_INFO, "Request Object.Reg result is %s\n", response.c_str());
}
void MangementModule::publishDeviceState(DeviceState* pDevice) {
	string message;
	Helper::toJsonMsgObjectState(*pDevice, message);
	sf::SubServicesManager::instance()->publishMessage(config.moduleId, sf::kSubscribeServiceObjectState, message);

}
void MangementModule::sendFakeUpdateMessage(DeviceState* pDevice, const char* value){
	string message;
	Helper::toJsonFakeMsgObjectState(*pDevice, value, message);
	sf::SubServicesManager::instance()->publishMessage(config.moduleId, sf::kSubscribeServiceObjectState, message);
	pDevice->mustReportStateTs = Helper::getCurrentSystemMS() + UNCONDITON_REPORT_TIME; //一分钟后无条件上报
}

DeviceState* MangementModule::findDeviceList(int network, int node) {
	//LOCK_DEVICE_LIST
	list<DeviceState*>::iterator it = pDeviceList.begin();
	for (; it != pDeviceList.end(); it++) {
		DeviceState* pNow = *it;
		if (pNow->network == network && pNow->node == node) {
			//UNLOCK_DEVICE_LIST
			return pNow;
		}
	}
	//UNLOCK_DEVICE_LIST
	return NULL;
}

void MangementModule::refreshDevicePhysicalState(bool updateName) {
	list<DevicePhysicalState*> devPhyList;
	getAllDevicePhyStates(devPhyList);

	//printf("\n\n=====================refresh from memory===============\n\n");
	// TODO debug
//	list<DevicePhysicalState*>::iterator debugIt = devPhyList.begin();
//	for(;debugIt!=devPhyList.end();debugIt++){
//            DevicePhysicalState* pTest = *debugIt;
//        printf("\tDevice (%d.%d) type-%d, ON/OFF=%d, online=%d\n", pTest->network, pTest->node, pTest->deviceType, pTest->simpleOnOff, pTest->online);
//	}
	// merge with currently in memory device state
	LOCK_DEVICE_LIST
	list<DeviceState*>::iterator it = pDeviceList.begin();
	for (; it != pDeviceList.end(); it++) {
		DeviceState* pDev = *it;
		DevicePhysicalState* pDevPhy = findPhyState(devPhyList, pDev->network, pDev->node);
		if (pDevPhy == NULL) {
			// the device was physically removed
			pDev->refreshResult = REFRESH_TYPE_DELETED;
			// Mark but not delete right now
			continue;
		}

		// TODO debug
		//printf("refreshDevicePhysicalState(): device %d.%d type is %d, on/off=%d, online=%d\n", pDevPhy->network,pDevPhy->node, pDevPhy->deviceType,pDevPhy->simpleOnOff, pDevPhy->online);

		if (pDevPhy->deviceType != pDev->deviceType || pDev->simpleOnOff != pDevPhy->simpleOnOff) {
			// 设备类型发生了变化
			if (pDevPhy->deviceType == DEVICE_UNKNOWN){
				//if (pDev->onLine != false){
					//pDev->refreshResult = REFRESH_TYPE_OFFLINE;
					logger(LEVEL_INFO, "Data changed: Device %d.%d offline. But not sure.", pDev->network, pDev->node);
				//}
			}else{
				pDev->refreshResult = REFRESH_TYPE_NEW;
				logger(LEVEL_INFO, "Data changed: Device %d.%d type changed %d=>%d", pDev->network, pDev->node, pDev->deviceType, pDevPhy->deviceType);
			}

			updateDeviceState(pDev, pDevPhy, updateName);
			if (pDev->refreshResult == REFRESH_TYPE_NEW){
                saveDeviceDatas();
			}

		} else if (!pDevPhy->online){
            continue;
		} else {
			string oldValue = pDev->value;
			string oldName = pDev->name;
			string oldRegion = pDev->region;
			bool oldOnLineState = pDev->onLine;
			updateDeviceState(pDev, pDevPhy, updateName);
			if (oldOnLineState != pDev->onLine ){
				logger(LEVEL_INFO, "Data changed: Device %d.%d online state changed. %d=>%d", pDev->network, pDev->node, oldOnLineState, pDev->onLine);
				pDev->refreshResult = REFRESH_TYPE_ONLINE;
			} else if (oldValue != pDev->value) {
                logger(LEVEL_INFO, "Data changed: Device %d.%d value changed. %s=>%s", pDev->network, pDev->node, oldValue.c_str(), pDev->value.c_str());
                pDev->mustReportStateTs = 0;
				pDev->refreshResult = REFRESH_TYPE_UPDATED;
			} else if (pDev->refreshResult != REFRESH_TYPE_SET_BY_CMD && pDev->refreshResult != REFRESH_TYPE_NEW) {
				pDev->refreshResult = REFRESH_TYPE_NOCHANGE;
			} else if (pDev->mustReportStateTs != 0 && Helper::timeIsUp(Helper::getCurrentSystemMS(), pDev->mustReportStateTs)){
				logger(LEVEL_INFO, "Data changed: Device %d.%d must report state", pDev->network, pDev->node);
				pDev->mustReportStateTs = 0;
				pDev->refreshResult = REFRESH_TYPE_UPDATED;
			}
			if (oldName != pDev->name || oldRegion != pDev->region){
                logger(LEVEL_INFO, "Data changed: Device %d.%d name changed. %s/%s=>%s/%s", pDev->network, pDev->node, oldRegion.c_str(), oldName.c_str(), pDev->region.c_str(), pDev->name.c_str());
                pDev->refreshResult = REFRESH_TYPE_SET_BY_CMD;
                saveDeviceDatas();
            }
		}
	}
	UNLOCK_DEVICE_LIST

	// find any new added device
	list<DevicePhysicalState*>::iterator it2 = devPhyList.begin();
	for (; it2 != devPhyList.end(); it2++) {
		DevicePhysicalState* pDevPhy = *it2;
		DeviceState* pDev = findDeviceList(pDevPhy->network, pDevPhy->node);
		if (pDev != NULL) {
			// already handled above
			continue;
		}
		pDev = createNewDeviceState();
		logger(LEVEL_INFO, "Found new device");
		pDev->refreshResult = REFRESH_TYPE_NEW;
		pDev->enabled = true;
		updateDeviceState(pDev, pDevPhy, updateName);
		LOCK_DEVICE_LIST
		if (addToDeviceList(pDev) == false){
			logger(LEVEL_INFO, "But the device already existed. Not added");
			delete pDev;
		}
		UNLOCK_DEVICE_LIST
	}

	// release all resource after all
	while (devPhyList.size() > 0) {
		DevicePhysicalState* pDevPhy = devPhyList.front();
		devPhyList.pop_front();
		delete pDevPhy;
	}
}

void MangementModule::updateDeviceState(DeviceState* pDev, DevicePhysicalState* pDevPhy, bool updateName) {
	pDev->network = pDevPhy->network;
	pDev->node = pDevPhy->node;
	if (pDev->manufacturer == "")
		pDev->manufacturer = config.manufacturer;

	if (pDev->id == "") {
		const char* pId = Helper::getIdByDeviceInfo(config.moduleId.c_str(), pDevPhy->network, pDevPhy->node);
		pDev->id = pId;
		delete pId;
	}

	if (updateName) {
        pDev->name = pDevPhy->name;
        pDev->region = pDevPhy->region;
	}

	char buff[20];
	pDev->onLine = pDevPhy->online;
	pDev->simpleOnOff = pDevPhy->simpleOnOff;
	// TODO debug
	//printf("udpate device when phy state(%d.%d) type is %d, simpleOnOff=%d\n", pDevPhy->network, pDevPhy->node, pDevPhy->deviceType, pDevPhy->simpleOnOff);

	if (pDevPhy->deviceType == DEVICE_LIGHT) {
		pDev->deviceType = DEVICE_LIGHT;
		pDev->level = pDevPhy->level;
		pDev->color = (pDevPhy->redColor << 16) | (pDevPhy->greenColor << 8) | (pDevPhy->blueColort);
		if (pDevPhy->simpleOnOff) {
			pDev->type = CONST_TYPE_LIGHT;
			if (pDev->subtype == "")
				pDev->subtype = "light";
			if (pDev->model == "")
				pDev->model = "DeltaDoreLight";
			pDev->value = pDev->level > 0 ? "on" : "off";
			pDev->cluster = CONST_CLUSTER_ONOFF;
		} else {
			pDev->type = CONST_TYPE_DIMMER;
			if (pDev->subtype == "")
				pDev->subtype = "dimmer";
			if (pDev->model == "")
				pDev->model = "DeltaDoreDimmer";
			sprintf(buff, "%d", pDev->level);
			pDev->value = buff;
			pDev->cluster = CONST_CLUSTER_LEVEL;
		}
	} else if (pDevPhy->deviceType == DEVICE_ROLLER_SHUTTER) {
		pDev->deviceType = DEVICE_ROLLER_SHUTTER;
		if (pDevPhy->position == 0 || pDevPhy->position == 1 || pDevPhy->position == 10 || pDevPhy->position == 11) {
			pDev->value = "0";
		} else if (pDevPhy->position == 100 || pDevPhy->position == 101) {
			pDev->value = "255";
		} else {
			pDev->value = "127";
		}
		pDev->type = CONST_TYPE_SHADE;
		pDev->cluster = CONST_CLUSTER_LEVEL; // any roller shutter recognized as LEVEL cluster.
		if (pDevPhy->simpleOnOff) {
			if (pDev->subtype == "")
				pDev->subtype = "3-state";
			if (pDev->model == "")
				pDev->model = "DeltaDoreRollerShutter3S";
		} else {
			if (pDev->subtype == "")
				pDev->subtype = "full-state";
			if (pDev->model == "")
				pDev->model = "DeltaDoreRollerShutterFS";
		}
	} else if (pDevPhy->deviceType == DEVICE_TEMPERATURE_SENSOR) {
		pDev->type = CONST_TYPE_TEMP_SENSOR;
		if (pDev->model == "")
			pDev->model = "DeltaDoreTemperatureSensor";
		sprintf(buff, "%3.1f", pDev->temperature);
		pDev->value = buff;
	} else if (pDevPhy->deviceType == DEVICE_UNKNOWN) {
	    // TODO debug
	    //printf("change online state of %s\n", pDev->id.c_str());
		pDev->onLine = false;
		//pDev->model = "DeltaDoreLight";
		//pDev->value = pDev->level > 0 ? "on" : "off";
		//pDev->cluster = CONST_CLUSTER_ONOFF;
	}
}

void MangementModule::stop() {
	started = false;
	stopPollingTimerThread();

	stopWebUICommandUdpListeningThread();
	stopMessageProcessingThread();
	stopDeviceCommandProcessingThread();
	stopDeviceDriver();
	sem_destroy(&semMessageList);
	sem_destroy(&semCommandList);
	sem_destroy(&semDeviceOperation);
	sem_destroy(&semDeviceList);
}

void MangementModule::releaseResource() {
	releaseDeviceResource();
	releaseWebUIUdpListenerResource();
	releaseManagerResource();
}

void MangementModule::stopDeviceDriver() {
    // nothing to do
}

void MangementModule::releaseDeviceResource() {
	// nothing to do
}

void MangementModule::releaseWebUIUdpListenerResource() {
	udpCommandManager.release();
}

void MangementModule::releaseManagerResource() {
	// release action-message and device-command list
	while (actionMessageList.size() > 0) {
		ActionNotifyMessage* pNotify = actionMessageList.front();
		actionMessageList.pop_front();
		delete pNotify->pMsg;
		pNotify->pMsg = NULL;
		delete pNotify;
	}
	logger(LEVEL_INFO, "DeltaDore actionMessageList cleared.");

	while (deviceCommandList.size() > 0) {
		DeviceActionCommand* pCmd = deviceCommandList.front();
		deviceCommandList.pop_front();
		delete pCmd;
	}
	logger(LEVEL_INFO, "DeltaDore deviceCommandList cleared.");

	// release all in memory device data
	saveDeviceDatas();
	while (pDeviceList.size() > 0) {
		DeviceState* pDev = pDeviceList.front();
		pDeviceList.pop_front();
		delete pDev;
	}
	logger(LEVEL_INFO, "DeltaDore deviceList cleared.");
}

void MangementModule::init() {
	sem_init(&semMessageList, 0, 1);
	sem_init(&semCommandList, 0, 1);
	sem_init(&semDeviceOperation, 0, 1);
	sem_init(&semDeviceList, 0, 1);
}

void MangementModule::getAllDeviceInfo(list<MsgObjectUpdate*>& devList) {
	list<MsgObjectUpdate*>::iterator it;
	LOCK_DEVICE_LIST
	for (it = pDeviceList.begin(); it != pDeviceList.end(); it++) {
        MsgObjectUpdate* pp = *it;
		devList.push_back(pp);
    }
	UNLOCK_DEVICE_LIST
}

/**************************** 外部接口相关函数 **********************************/

/**
 * only schedule commands here, not do actual action
 */
void MangementModule::onSetValueAction(MsgActionExecute* pMsg) {
	ActionNotifyMessage * pNotify = new ActionNotifyMessage();
	pNotify->delayTimeMs = 0;
	pNotify->startTimeMs = Helper::getCurrentSystemMS();
	pNotify->pMsg = pMsg;
	sem_wait(&semMessageList);
	actionMessageList.push_back(pNotify);
	sem_post(&semMessageList);
}

/**
 * basically, only use this to save icon, cluster, region, name ...
 */
#define TAKE_FIELD(name) \
	if (msg.name != CONST_MSG_FIELD_NOT_SET) { \
		pDev->name = msg.name; \
	}

int MangementModule::addNewDevice(int network, MsgObjectUpdate& msg, string& errMsg)
{
    cJSON* pResult = NULL;
    sem_wait(&semDeviceOperation);
    pResult = deltadoreDriver.registerNode(network);
    sem_post(&semDeviceOperation);

    cJSON * pJsonSuccess = cJSON_GetObjectItem(pResult, "success");
    if (pJsonSuccess->type == cJSON_False){
        // add new node failed
        pJsonSuccess = cJSON_GetObjectItem(pResult, "message");
        errMsg = pJsonSuccess->valuestring;
        cJSON_Delete(pResult);
        return STATUS_BAD_RREQUEST;
    }

    // add new node success. find it out and try to set the name and region
    list<DevicePhysicalState*> devPhyStates;
	getAllDevicePhyStates(devPhyStates);
    list<DevicePhysicalState*>::iterator existedDeviceIt = devPhyStates.begin();
    for(;existedDeviceIt != devPhyStates.end();existedDeviceIt++){
        DevicePhysicalState* pPhyDev = *existedDeviceIt;
        if (findDeviceList(pPhyDev->network, pPhyDev->node) == NULL){
            DeviceActionCommand command;
            command.commandCode = CMD_SET_DEVICE_NAME;
            command.network = pPhyDev->network;
            command.node = pPhyDev->node;
            command.region = msg.region;
            command.name = msg.name;
            DevicePhysicalState result;
            executeDeviceCommand(&command, &result);


            DeviceState* pNewDevice = createNewDeviceState();
            pNewDevice->enabled = true; // init default value
            pNewDevice->network = pPhyDev->network;
            pNewDevice->node = pPhyDev->node;
            pNewDevice->id = msg.id;
            pNewDevice->icon = msg.icon;
            pNewDevice->name = msg.name;
            pNewDevice->region = msg.region;
            pNewDevice->enabled = msg.enabled;
			pNewDevice->mustReportStateTs = 0;
            if (!msg.enabled){
                logger(LEVEL_INFO, "device %d.%d added with enabled=false", pPhyDev->network, pPhyDev->node);
            }
            pNewDevice->refreshResult = REFRESH_TYPE_NEW;

			LOCK_DEVICE_LIST
			if (addToDeviceList(pNewDevice) == false){
				logger(LEVEL_INFO, "But the device already existed. Not added");
				delete pNewDevice;
			}
			saveDeviceDatas();
			UNLOCK_DEVICE_LIST
            errMsg = "success";
        }
        delete pPhyDev;
    }
    cJSON_Delete(pResult);
    return SUCCESS;
}

int MangementModule::onObjectUpdateRequest(MsgObjectUpdate& msg, string& errMsg) {
	DeviceState* pDev = findDeviceFromMessage(msg.id);
	if (pDev == NULL) {
		errMsg = "Device required to update not existed, try to add new one automatically";
		int validNetworkId = -1;
		for(int i=0;i<MAX_NETWORK_NUM;i++){
            for(int j=0;j<MAX_NODE_NUM/2;j++){
                if (findDeviceList(i, j) == NULL){
                    validNetworkId = i;
                    break;
                }
            }
            if (validNetworkId >= 0){
                break;
            }
		}
		return addNewDevice(validNetworkId, msg, errMsg);

	}
	TAKE_FIELD(cluster)
	TAKE_FIELD(region)
	TAKE_FIELD(name)
	TAKE_FIELD(icon)
	logger(LEVEL_INFO, "msg enabled was set to %b", msg.enabled);
	pDev->enabled = msg.enabled;
	pDev->refreshResult = REFRESH_TYPE_SET_BY_CMD;

	DeviceActionCommand command;
	command.commandCode = CMD_SET_DEVICE_NAME;
	command.network = pDev->network;
	command.node = pDev->node;
	command.region = msg.region;
	command.name = msg.name;
	DevicePhysicalState result;
	executeDeviceCommand(&command, &result);
	saveDeviceDatas();
	return SUCCESS;
}
int MangementModule::onObjectDeleteRequest(MsgObjectDelete& msg, string& errMsg) {
    DeviceState* pDev = findDeviceFromMessage(msg.id);
    if (pDev == NULL){
        errMsg = "Device id not existed";
        return STATUS_BAD_RREQUEST;
    }
    deltadoreDriver.deleteNode(pDev->network, pDev->node);
    // polling routine will call refreshDevicePhysicalState();
	return SUCCESS;
}

DeviceState* MangementModule::findDeviceFromMessage(const string& id) {
	list<DeviceState*>::iterator it;
	//LOCK_DEVICE_LIST
	for (it = pDeviceList.begin(); it != pDeviceList.end(); it++) {
		DeviceState* pDevice = *it;
		if (pDevice->id == id) {
			//UNLOCK_DEVICE_LIST
			return pDevice;
		}
	}
	//UNLOCK_DEVICE_LIST
	return NULL;
}

void MangementModule::executeDeviceCommand(DeviceActionCommand* pCommand, DevicePhysicalState* pResult) {
	cJSON* pJsonResult = NULL;
	DevicePhysicalState phyStateResult;

	sem_wait(&semDeviceOperation);
	switch (pCommand->commandCode) {
	case CMD_SET_LIGHT_ON: {
		logger(LEVEL_DEBUG, "call switchOnLight(%d,%d)", pCommand->network, pCommand->node);
		pJsonResult = deltadoreDriver.switchOnLight(pCommand->network, pCommand->node);
		handleCommandResult(pCommand, pJsonResult);
		break;
	}
	case CMD_SET_LIGHT_OFF: {
		logger(LEVEL_DEBUG, "call switchOffLight(%d,%d)", pCommand->network, pCommand->node);
		pJsonResult = deltadoreDriver.switchOffLight(pCommand->network, pCommand->node);
		handleCommandResult(pCommand, pJsonResult);
		break;
	}
	case CMD_SET_LIGHT_LEVEL: {
		logger(LEVEL_DEBUG, "call setLightLevel(%d,%d,%d)", pCommand->network, pCommand->node, pCommand->intParam1);
		pJsonResult = deltadoreDriver.setLightLevel(pCommand->network, pCommand->node, pCommand->intParam1);
		handleCommandResult(pCommand, pJsonResult);
		break;
	}
	case CMD_SET_LIGHT_TOGGLE: {
		logger(LEVEL_DEBUG, "call toggleLight(%d,%d)", pCommand->network, pCommand->node);
		pJsonResult = deltadoreDriver.toggleLight(pCommand->network, pCommand->node);
		handleCommandResult(pCommand, pJsonResult);
		break;
	}
	case CMD_GET_LIGHT_STATE: {
		logger(LEVEL_DEBUG, "call queryLightPhyState(%d,%d)", pCommand->network, pCommand->node);
        int done = deltadoreDriver.queryLightPhyState(pCommand->network, pCommand->node, phyStateResult);
        DeviceState* pDev = findDeviceList(phyStateResult.network, phyStateResult.node);
		if (done == 1) {
            if (pDev == NULL){
                pDev = createNewDeviceState();
                pDev->enabled = true;
				pDev->network = pCommand->network;
				pDev->node = pCommand->node;
				pDev->mustReportStateTs = 0;
				LOCK_DEVICE_LIST
				if (addToDeviceList(pDev) == false){
					logger(LEVEL_INFO, "But the device already existed. Not added");
					delete pDev;
				}
				UNLOCK_DEVICE_LIST

            }
            //updateDeviceState(pDev, &phyStateResult);
		}else if (done == 2){
			deltadoreDriver.getNodeRealType(pCommand->network, pCommand->node, phyStateResult);
		}
		break;
	}
	case CMD_SET_ROLLERSHUTTER_OPEN: {
		logger(LEVEL_DEBUG, "call openRollerShutter(%d,%d)", pCommand->network, pCommand->node);
		pJsonResult = deltadoreDriver.openRollerShutter(pCommand->network, pCommand->node);
		handleCommandResult(pCommand, pJsonResult);
		break;
	}
	case CMD_SET_ROLLERSHUTTER_CLOSE: {
		logger(LEVEL_DEBUG, "call closeRollerShutter(%d,%d)", pCommand->network, pCommand->node);
		pJsonResult = deltadoreDriver.closeRollerShutter(pCommand->network, pCommand->node);
		handleCommandResult(pCommand, pJsonResult);
		break;
	}
	case CMD_SET_ROLLERSHUTTER_STOP: {
		logger(LEVEL_DEBUG, "call stopRollerShutter(%d,%d)", pCommand->network, pCommand->node);
		pJsonResult = deltadoreDriver.stopRollerShutter(pCommand->network, pCommand->node);
		handleCommandResult(pCommand, pJsonResult);
		break;
	}
	case CMD_GET_ROLLERSHUTTER_STATE: {
		logger(LEVEL_DEBUG, "call queryRoolershutterPhyState(%d,%d)", pCommand->network, pCommand->node);
		int done = deltadoreDriver.queryRoolershutterPhyState(pCommand->network, pCommand->node, phyStateResult);
		if (done == 1) {

            DeviceState* pDev = findDeviceList(phyStateResult.network, phyStateResult.node);
            if (pDev == NULL){
                pDev = createNewDeviceState();
                pDev->enabled = true;
				pDev->network = pCommand->network;
				pDev->node = pCommand->node;
				pDev->mustReportStateTs = 0;
				LOCK_DEVICE_LIST
				if (addToDeviceList(pDev) == false){
					logger(LEVEL_INFO, "But the device already existed. Not added");
					delete pDev;
				}
				UNLOCK_DEVICE_LIST

            }
            //updateDeviceState(pDev, &phyStateResult);
		}else if (done == 2){
			deltadoreDriver.getNodeRealType(pCommand->network, pCommand->node, phyStateResult);
		}
		break;
	}
	case CMD_SET_DEVICE_NAME:{
	    logger(LEVEL_DEBUG, "call setNodeName(%d,%d,%s,%s)", pCommand->network, pCommand->node, pCommand->region.c_str(), pCommand->name.c_str());
		deltadoreDriver.setNodeName(pCommand->network, pCommand->node, pCommand->region.c_str(), pCommand->name.c_str());
		break;
	}
	case CMD_REFRESH_DEVICE_STATE:{
	    logger(LEVEL_DEBUG, "call getNodeRealType(%d,%d)", pCommand->network, pCommand->node);
		bool done = deltadoreDriver.getNodeRealType(pCommand->network, pCommand->node, phyStateResult);
		if (done) {
            DeviceState* pDev = findDeviceList(phyStateResult.network, phyStateResult.node);
            if (pDev == NULL){
                pDev = createNewDeviceState();
                pDev->enabled = true;
				pDev->network = pCommand->network;
				pDev->node = pCommand->node;
				pDev->mustReportStateTs = 0;

				LOCK_DEVICE_LIST
				if (addToDeviceList(pDev) == false){
					logger(LEVEL_INFO, "But the device already existed. Not added");
					delete pDev;
				}
				UNLOCK_DEVICE_LIST
            }
            //updateDeviceState(pDev, &phyStateResult);
		}
		break;
	}
	default: {
		printf(
				"======================================\ncommand need more code\n=======================================\n");
	}
	}
	sem_post(&semDeviceOperation);
	if (pJsonResult != NULL){
		cJSON_Delete(pJsonResult);
	}
}

/**
 * Note: this is read the cached device statues.
 */
void MangementModule::getAllDevicePhyStates(list<DevicePhysicalState*>& deviceStateList) {
	sem_wait(&semDeviceOperation);
	deltadoreDriver.getAllDevicePhyStates(deviceStateList);
	sem_post(&semDeviceOperation);
}

bool MangementModule::addToDeviceList(DeviceState* pNewDeviceState){
    printf("  Add device %d.%d into list, type=%d, online=%d\n\n", pNewDeviceState->network, pNewDeviceState->node, pNewDeviceState->deviceType, pNewDeviceState->onLine);
	list<DeviceState*>::iterator it = pDeviceList.begin();
	bool existed = false;
	for(;it != pDeviceList.end(); it++){
		DeviceState* pDev = *it;
		if (pDev->network == pNewDeviceState->network && pDev->node == pNewDeviceState->node){
			existed = true;
			break;
		}
	}
	if (existed){
		return false;
	}
	pDeviceList.push_back(pNewDeviceState);
	return true;
}

void MangementModule::handleCommandResult(DeviceActionCommand* pCommand, cJSON* pJsonResult){
    cJSON* pSuccess = cJSON_GetObjectItem(pJsonResult, "success");
    if (pSuccess != NULL && pSuccess->type == cJSON_True){
        return;
    }
    DeviceState* pDevice = findDeviceList(pCommand->network, pCommand->node);
    if (pDevice == NULL){
        return;
    }
    //pDevice->mustReportStateTs = Helper::getCurrentSystemMS();
}

}
