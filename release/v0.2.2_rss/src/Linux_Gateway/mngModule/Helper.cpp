/*
 * Helper.cpp
 *
 *  Created on: Dec 17, 2016
 *      Author: clari
 */

#include "Helper.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sys/time.h>
#include <errno.h>

using namespace std;

#define GET_DEV_CFG(name) \
		pDevData = cJSON_GetObjectItem(pDev, #name); 			\
		if (pDevData == NULL){									\
			cfg->name = "";										\
		}else{													\
			cfg->name = pDevData->valuestring;					\
		}														\
		logger(LEVEL_INFO, "    "#name"=%s", cfg->name.c_str())
#define GET_DEV_CFGINT(name) \
		pDevData = cJSON_GetObjectItem(pDev, #name); 			\
		if (pDevData == NULL){									\
			cfg->name = -1;										\
		}else{													\
			cfg->name = pDevData->valueint;						\
		}														\
		logger(LEVEL_INFO, "    "#name"=%d", cfg->name)

#define GET_CONFIG_STRING(name) \
		pData = cJSON_GetObjectItem(pJson, #name);	\
		if (pData != NULL){	\
			config.name = pData->valuestring;	\
			logger(LEVEL_INFO, "Configured "#name" is [%s]", config.name.c_str());	\
		}

#define GET_CONFIG_INT(name) \
		pData = cJSON_GetObjectItem(pJson, #name);	\
		if (pData != NULL){	\
			config.name = pData->valueint;	\
			logger(LEVEL_INFO, "Configured "#name" is [%d]", config.name);	\
		}

bool getJsonBoolean(cJSON* pData) {
	if (pData->type == cJSON_True) {
		return true;
	} else if (pData->type == cJSON_False) {
		return false;
	} else if (pData->type == cJSON_Number) {
		return pData->valueint != 0;
	} else if (pData->type == cJSON_String) {
		string strVal = pData->valuestring;
		if (strVal == "1" || strVal == "true") {
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}
void Helper::parseConfigFromJson(const char* jsonStr, ModuleConfig& config) {
	cJSON * pJson = cJSON_Parse(jsonStr);
	if (NULL == pJson) {
		return;
	}
	cJSON * pData;
	GET_CONFIG_STRING(moduleId)
	GET_CONFIG_STRING(deviceName)
	GET_CONFIG_STRING(manufacturer)
	GET_CONFIG_INT(webCommandPort)
}

void Helper::parseDeviceConfigFromJson(const char* jsonStr, list<DeviceState*>& deviceConfigs) {
	deviceConfigs.clear();
	cJSON * pData = cJSON_Parse(jsonStr);
	if (pData == NULL) {
		logger(LEVEL_INFO, "No any configured device");
		return;
	}
	if (pData->type != cJSON_Array) {
		logger(LEVEL_ERROR, "invalid device data. It must be array");
		EXIT_WITH_ERROR(ERR_CODE_LOADCONFIG);
	}
	int len = cJSON_GetArraySize(pData);
	logger(LEVEL_INFO, "Configured %d devices", len);

	for (int i = 0; i < len; i++) {
		cJSON* pDev = cJSON_GetArrayItem(pData, i);
		assert(pDev != NULL);
		DeviceState* cfg = new DeviceState();
		cfg->onLine = false;
		cfg->deviceType = DEVICE_UNKNOWN;
		cfg->enabled = true; // default is enabled
		cJSON* pDevData = cJSON_GetObjectItem(pDev, "id");
		if (pDevData == NULL || pDevData->valuestring == NULL) {
			logger(LEVEL_ERROR, "invalid device ID config.");
			EXIT_WITH_ERROR(ERR_CODE_LOADCONFIG);
		}
		cfg->id = pDevData->valuestring;
		logger(LEVEL_INFO, "  id=%s", cfg->id.c_str());
		GET_DEV_CFG(name);
		GET_DEV_CFG(region);
		pDevData = cJSON_GetObjectItem(pDev, "enabled");
		if (pDevData == NULL) {
			cfg->enabled = true;
		} else {
			cfg->enabled = getJsonBoolean(pDevData);
			if (!cfg->enabled){
                logger(LEVEL_INFO, "Cfg device %s with enabled=false", cfg->id.c_str());
			}
		}
		GET_DEV_CFG(icon);
		GET_DEV_CFG(cluster);
		GET_DEV_CFGINT(network);
		GET_DEV_CFGINT(node);
		GET_DEV_CFG(type);

		deviceConfigs.push_back(cfg);
	}
}

int Helper::lastIndexOf(const char* string, const char* find) {
	int strLen = strlen(string);
	int findLen = strlen(find);
	int startPosition = strLen - findLen;
	if (startPosition < 0) {
		return -1;
	}
	bool found = false;
	for (int i = startPosition; i >= 0; i--) {
		found = true;
		for (int j = 0; j < findLen; j++) {
			if (string[i + j] != find[j]) {
				found = false;
				break;
			}
		}
		if (found) {
			return i;
		}
	}
	return -1;
}

#define GOTO_NEXT_TOKEN(p) while(*p != '_') p++; p++;
void Helper::getNetworkAndNodeFromId(const char* id, int& network, int& node) {
	const char* pTemp = id;
	GOTO_NEXT_TOKEN(pTemp);
	sscanf(pTemp, "%d", &network);
	GOTO_NEXT_TOKEN(pTemp);
	sscanf(pTemp, "%d", &node);
	printf("network = %d, node =%d\n", network, node);
}
#undef GOTO_NEXT_TOKEN

char* Helper::getIdByDeviceInfo(const char* moduleId, int network, int node) {
	char* strId = new char[strlen(moduleId) + 12];
	sprintf(strId, "%s_%d_%d", moduleId, network, node);
	return strId;
}

#define GET_MSG_MEMBER_STRING(name, nullable, defVal) \
		pData = cJSON_GetObjectItem(pJson, #name); \
		if (pData == NULL){ \
			if (!nullable){ \
				throw DeltaDoreException(ERR_CODE_MSG_MEMBER_NULL); \
			}else{ \
				msg.name = (const char*)defVal; \
			} \
		}else{ \
			msg.name = pData->valuestring; \
		}

void Helper::parseMsgObjectUpdateFromJson(cJSON* pJson, MsgObjectUpdate& msg) throw (DeltaDoreException) {
	cJSON* pData;

	GET_MSG_MEMBER_STRING(id, false, NULL)
	GET_MSG_MEMBER_STRING(type, true, CONST_MSG_FIELD_NOT_SET)
	GET_MSG_MEMBER_STRING(subtype, true, CONST_MSG_FIELD_NOT_SET)
	pData = cJSON_GetObjectItem(pJson, "enabled");
	if (pData == NULL) {
		msg.enabled = true;
	} else {
		msg.enabled = getJsonBoolean(pData);
	}
	GET_MSG_MEMBER_STRING(name, true, CONST_MSG_FIELD_NOT_SET)
	GET_MSG_MEMBER_STRING(cluster, true, CONST_MSG_FIELD_NOT_SET)
	GET_MSG_MEMBER_STRING(region, true, CONST_MSG_FIELD_NOT_SET)
	GET_MSG_MEMBER_STRING(icon, true, CONST_MSG_FIELD_NOT_SET)
	GET_MSG_MEMBER_STRING(manufacturer, true, CONST_MSG_FIELD_NOT_SET)
	GET_MSG_MEMBER_STRING(model, true, CONST_MSG_FIELD_NOT_SET)
	getNetworkAndNodeFromId(msg.id.c_str(), msg.network, msg.node);

	logger(LEVEL_DEBUG, "get message Object.Update:\n"
			"    id=%s\n    type=%s\n    subtype=%s\n    enabled=%s\n"
			"    name=%s\n    region=%s\n    icon=%s\n    cluster=%s\n    manufacturer=%s\n    model=%s\n",
			msg.id.c_str(), msg.type.c_str(), msg.subtype.c_str(), msg.enabled ? "true" : "false", msg.name.c_str(),
			msg.region.c_str(), msg.icon.c_str(), msg.cluster.c_str(), msg.manufacturer.c_str(), msg.model.c_str());
}

void Helper::parseMsgObjectDeleteFromJson(cJSON* pJson, MsgObjectDelete& msg) throw (DeltaDoreException) {
	cJSON* pData;
	GET_MSG_MEMBER_STRING(id, false, NULL)
	logger(LEVEL_DEBUG, "get message Object.Delete:\n    id=%s\n", msg.id.c_str());
}
void Helper::parseMsgActionExecute(cJSON* pJson, MsgActionExecute& msg) throw (DeltaDoreException) {
	cJSON* pData;

	GET_MSG_MEMBER_STRING(type, false, NULL)
	if (msg.type == "repeat") {
		msg.action_type = ACTION_TYPE_Repeat;
		GET_MSG_MEMBER_STRING(period, false, NULL)
		msg.value_period = atoi(msg.period.c_str());
		GET_MSG_MEMBER_STRING(count, false, NULL);
		msg.value_count = atoi(msg.count.c_str());
		cJSON* pActions = cJSON_GetObjectItem(pJson, "actions");
		if (pActions == NULL) {
			logger(LEVEL_INFO, "No any actions in message");
			throw DeltaDoreException(ERR_CODE_MSG_ACTIONS_NULL);
		}
		if (pActions->type != cJSON_Array) {
			logger(LEVEL_ERROR, "invalid actions in message. It must be array");
			throw DeltaDoreException(ERR_CODE_MSG_INVALID_ACTIONS);
		}
		int len = cJSON_GetArraySize(pActions);
		if (len < 1) {
			logger(LEVEL_ERROR, "invalid actions in message. It must not be empty");
			throw DeltaDoreException(ERR_CODE_MSG_INVALID_ACTIONS);
		}
		for (int i = 0; i < len; i++) {
			cJSON * pItem = cJSON_GetArrayItem(pActions, i);
			MsgActionExecute* actionMsg = new MsgActionExecute();
			Helper::parseSimpleMsgActionExecute(pItem, *actionMsg);
			msg.repeatActions.push_back(actionMsg);
		}
	} else {
		Helper::parseSimpleMsgActionExecute(pJson, msg);
	}
}
void setMsgValue(MsgActionExecute& msg) {
	if (msg.value == "0") {
		msg.value_level = 0;
		msg.value_onOff = false;
	} else if (msg.value == "1") {
		msg.value_level = 1;
		msg.value_onOff = true;
	} else if (msg.value == "off") {
		msg.value_onOff = false;
		msg.value_level = -1;
	} else if (msg.value == "on") {
		msg.value_onOff = true;
		msg.value_level = -1;
	} else if (msg.value == "close" || msg.value == "open" || msg.value == "stop") {
		msg.value_level = -1;
	} else {
		msg.value_level = atoi(msg.value.c_str());
	}
}
void Helper::parseSimpleMsgActionExecute(cJSON* pJson, MsgActionExecute& msg) throw (DeltaDoreException) {
	cJSON* pData;

	GET_MSG_MEMBER_STRING(type, false, NULL)
	if (msg.type == "set-value") {
		GET_MSG_MEMBER_STRING(id, false, NULL)
		GET_MSG_MEMBER_STRING(value, false, NULL)
		msg.action_type = ACTION_TYPE_SetValue;
		setMsgValue(msg);
	}else if (msg.type == "setall-value") {
		GET_MSG_MEMBER_STRING(clusterid, false, NULL)
		GET_MSG_MEMBER_STRING(value, false, NULL)
		msg.action_type = ACTION_TYPE_SetAllValue;
		setMsgValue(msg);
	} else if (msg.type == "toggle-value") {
		GET_MSG_MEMBER_STRING(id, false, NULL)
		msg.action_type = ACTION_TYPE_toggleValue;
	} else if (msg.type == "send-read-request") {
		GET_MSG_MEMBER_STRING(id, false, NULL)
		msg.action_type = ACTION_TYPE_refreshState;
	} else if (msg.type == "cycle-on-off") {
		GET_MSG_MEMBER_STRING(id, false, NULL)
		msg.action_type = ACTION_TYPE_cycleOnOff;
		GET_MSG_MEMBER_STRING(count, false, NULL)
		GET_MSG_MEMBER_STRING(delay_on, false, NULL)
		GET_MSG_MEMBER_STRING(delay_off, false, NULL)
		msg.value_count = atoi(msg.count.c_str());
		msg.value_delay_on = atoi(msg.delay_on.c_str());
		msg.value_delay_off = atoi(msg.delay_off.c_str());
	} else if (msg.type == "delay-off") {
		GET_MSG_MEMBER_STRING(id, false, NULL)
		msg.action_type = ACTION_TYPE_delayOff;
		GET_MSG_MEMBER_STRING(delaytime, false, NULL)
		msg.value_delaytime = atoi(msg.delaytime.c_str());
	} else if (msg.type == "set-enable") {
		GET_MSG_MEMBER_STRING(id, false, NULL)
		msg.action_type = ACTION_TYPE_setEnable;
		GET_MSG_MEMBER_STRING(value, false, NULL)
		msg.value_enable = getJsonBoolean(pData);
	} else if (msg.type == "query-objects") {
		msg.action_type = ACTION_TYPE_queryObjects;
	} else if (msg.type == "repeat") {
		logger(LEVEL_ERROR, "Unsupport nested repeat message");
		throw DeltaDoreException(ERR_CODE_MSG_NESTED_REPEAT);
	} else {
		logger(LEVEL_ERROR, "Unsupport type in message: %s", msg.type.c_str());
		throw DeltaDoreException(ERR_CODE_MSG_UNSUPPORT_TYPE);
	}
}

void Helper::listMsgObjectUpdateToJson(list<MsgObjectUpdate*>& deviceList, const string& serverName, string& jsonStr) {
	if (deviceList.size() == 0) {
		jsonStr = "{\"Method\":\"ObjectList.Reg\",\"Param\":{\"objects\":[],\"server\":\"DeltaDoreDriver\"}}";
		return;
	}
	cJSON * root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Method", "ObjectList.Reg");
	cJSON * j_param = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "Param", j_param);
	cJSON * objects = cJSON_CreateArray();
	cJSON_AddItemToObject(j_param, "objects", objects);
	cJSON_AddStringToObject(j_param, "server", serverName.c_str());

	list<MsgObjectUpdate*>::iterator it;
	for (it = deviceList.begin(); it != deviceList.end(); it++) {
        MsgObjectUpdate* pDev = *it;
            printf("JSON device %d.%d type=%s, deviceType=%d, online=%d\n", pDev->network, pDev->node, pDev->type.c_str(), pDev->deviceType, pDev->onLine);

		cJSON * pData = cJSON_CreateObject();
		cJSON_AddItemToArray(objects, pData);

		cJSON_AddStringToObject(pData, "id", pDev->id.c_str());
		bool isUnknownType = false;
		if (pDev->type != CONST_TYPE_DIMMER && pDev->type != CONST_TYPE_LIGHT && pDev->type != CONST_TYPE_SHADE ){
		    cJSON_AddStringToObject(pData, "type", CONST_TYPE_LIGHT);
		    isUnknownType = true;
		}else{
            cJSON_AddStringToObject(pData, "type", pDev->type.c_str());
		}
		cJSON_AddStringToObject(pData, "subtype", pDev->subtype.c_str());
		cJSON_AddStringToObject(pData, "enabled", pDev->enabled?"true":"false");
		cJSON_AddStringToObject(pData, "name", pDev->name.c_str());
		cJSON_AddStringToObject(pData, "region", pDev->region.c_str());
		cJSON_AddStringToObject(pData, "icon", pDev->icon.c_str());
		cJSON_AddStringToObject(pData, "cluster", pDev->cluster.c_str());
		cJSON_AddStringToObject(pData, "manufacturer", pDev->manufacturer.c_str());
		cJSON_AddStringToObject(pData, "model", pDev->model.c_str());
		if (pDev->deviceType != DEVICE_UNKNOWN || !isUnknownType){
            cJSON_AddStringToObject(pData, "online", "1");
		}else{
		    cJSON_AddStringToObject(pData, "online", "0");
		}

	}

	char* pJsonStr = cJSON_PrintUnformatted(root);
	jsonStr = pJsonStr;
	free(pJsonStr);
	cJSON_Delete(root);
	logger(LEVEL_DEBUG, "create MsgObjectUpdate messag: %s\n", jsonStr.c_str());
}

void Helper::toJsonMsgObjectState(MsgObjectUpdate& msg, string& jsonStr){
	cJSON * root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "id", msg.id.c_str());
	cJSON_AddStringToObject(root, "enabled", msg.enabled?"true":"false");
	cJSON_AddStringToObject(root, "value", msg.value.c_str());
	cJSON_AddStringToObject(root, "battery", "255");
	if (msg.onLine){
		cJSON_AddStringToObject(root, "LQI", "50");
	}else{
		cJSON_AddStringToObject(root, "LQI", "0");	// 如果离线了，只报信号无，不报离线
	}
	char* pJsonStr = cJSON_PrintUnformatted(root);
	jsonStr = pJsonStr;
	free(pJsonStr);
	logger(LEVEL_DEBUG, "create /object/state message: %s\n", jsonStr.c_str());
}

void Helper::toJsonFakeMsgObjectState(MsgObjectUpdate& msg, const char* value, string& jsonStr){
	cJSON * root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "id", msg.id.c_str());
	cJSON_AddStringToObject(root, "enabled", msg.enabled?"true":"false");
	cJSON_AddStringToObject(root, "value", value);
	cJSON_AddStringToObject(root, "battery", "255");
	if (msg.onLine){
		cJSON_AddStringToObject(root, "LQI", "50");
	}else{
		cJSON_AddStringToObject(root, "LQI", "0");	// 如果离线了，只报信号无，不报离线
	}
	char* pJsonStr = cJSON_PrintUnformatted(root);
	jsonStr = pJsonStr;
	free(pJsonStr);
	logger(LEVEL_DEBUG, "create fake /object/state message: %s\n", jsonStr.c_str());
}

void Helper::toJsonReqObjectUnReg(string& id, string& jsonStr){
	cJSON * root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Method", "Object.UnReg");
	cJSON * j_param = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "Param", j_param);
	cJSON_AddStringToObject(j_param, "id", id.c_str());
	char* pJsonStr = cJSON_PrintUnformatted(root);
	jsonStr = pJsonStr;
	free(pJsonStr);
	logger(LEVEL_DEBUG, "create Object.UnReg request: %s\n", jsonStr.c_str());
}
void Helper::toJsonRequestObjectReg(MsgObjectUpdate* pDevice, const string& serverName, string& jsonStr) {
	assert(pDevice != NULL);
	cJSON * root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Method", "Object.Reg");
	cJSON * pData = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "Param", pData);

	cJSON_AddStringToObject(pData, "id", pDevice->id.c_str());

    printf("JSON device %d.%d type=%s, deviceType=%d, online=%d\n", pDevice->network, pDevice->node, pDevice->type.c_str(), pDevice->deviceType, pDevice->onLine);

	if (pDevice->deviceType == DEVICE_UNKNOWN){
		cJSON_AddStringToObject(pData, "type", CONST_TYPE_LIGHT);
		cJSON_AddStringToObject(pData, "subtype", "");
	}else{
		cJSON_AddStringToObject(pData, "type", pDevice->type.c_str());
		cJSON_AddStringToObject(pData, "subtype", pDevice->subtype.c_str());
	}
	if (pDevice->type == CONST_TYPE_LIGHT || pDevice->type == CONST_TYPE_DIMMER || pDevice->type == CONST_TYPE_SHADE){
            cJSON_AddStringToObject(pData, "online", "1");
		}else{
		    cJSON_AddStringToObject(pData, "online", "0");
		}
	cJSON_AddStringToObject(pData, "enabled", pDevice->enabled?"true":"false");
	cJSON_AddStringToObject(pData, "name", pDevice->name.c_str());
	cJSON_AddStringToObject(pData, "region", pDevice->region.c_str());
	cJSON_AddStringToObject(pData, "icon", pDevice->icon.c_str());
	cJSON_AddStringToObject(pData, "cluster", pDevice->cluster.c_str());
	cJSON_AddStringToObject(pData, "manufacturer", pDevice->manufacturer.c_str());
	cJSON_AddStringToObject(pData, "model", pDevice->model.c_str());


	char* pJsonStr = cJSON_PrintUnformatted(root);
	jsonStr = pJsonStr;
	free(pJsonStr);
	cJSON_Delete(root);
	logger(LEVEL_DEBUG, "create MsgObjectUpdate messag: %s\n", jsonStr.c_str());
}

TIME_TS Helper::getCurrentSystemMS() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (TIME_TS) ((tv.tv_sec) * 1000 + (tv.tv_usec) / 1000);
}

bool Helper::timeIsUp(TIME_TS curTs, TIME_TS plannedTs) {
	TIME_TS deltaTs = curTs - plannedTs;
	if (deltaTs >= 0 && deltaTs < 0x7FFFFFFF) {
		return true;
	} else {
		return false;
	}
}

cJSON* Helper::toJsonDeviceDatas(list<MsgObjectUpdate*>& deviceList) {
	cJSON* pArray = cJSON_CreateArray();
	list<MsgObjectUpdate*>::iterator it = deviceList.begin();
	for(;it != deviceList.end();it++){
		cJSON* pData = cJSON_CreateObject();
		MsgObjectUpdate* pDevice = *it;
		printf("Save %s, type=%s\n", pDevice->id.c_str());
		cJSON_AddStringToObject(pData, "id", pDevice->id.c_str());
        cJSON_AddStringToObject(pData, "type", pDevice->type.c_str());
		cJSON_AddStringToObject(pData, "name", pDevice->name.c_str());
		cJSON_AddStringToObject(pData, "region", pDevice->region.c_str());
		cJSON_AddStringToObject(pData, "icon", pDevice->icon.c_str());
		cJSON_AddStringToObject(pData, "cluster", pDevice->cluster.c_str());
		cJSON_AddNumberToObject(pData, "network", pDevice->network);
		cJSON_AddNumberToObject(pData, "node", pDevice->node);
		cJSON_AddItemToArray(pArray, pData);
	}
	return pArray;
}

void Helper::sleep_ms(TIME_TS mSec) {
	struct timeval tv;
	tv.tv_sec = mSec / 1000;
	tv.tv_usec = (mSec % 1000) * 1000;
	int err;
	do {
		err = select(0, NULL, NULL, NULL, &tv);
	} while (err < 0 && errno == EINTR);
}
