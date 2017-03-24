//
//  ExtService.cpp
//  ModuleSample
//
//  Created by hu_danyuan on 16/12/02.
//  Copyright © 2016年 杭州速凡网络科技有限公司. All rights reserved.
//

#include "DtdMngService.h"
#include <stdlib.h>
#include <stdio.h>

#include "SF_ModBridge/ExtServicesManager.h"

using namespace cla;
using namespace std;


#define ERR_NO_ERROR 0
#define ERR_INVALID_JSON 1
#define ERR_NO_METHOD_REQUEST 2
#define ERR_NO_PARAM_REQUEST 3
#define ERR_METHOD_NOT_ALLOW 4

int makeResponse(cJSON* pJson, string& response, int statuesCode, int responseCode, const char* info){
	cJSON* root=cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "Result", responseCode);
	cJSON_AddStringToObject(root, "Info", info);
	char* pJsonStr = cJSON_PrintUnformatted(root);
	response = pJsonStr;
	cJSON_Delete(root);
	free(pJsonStr);
	if (pJson != NULL){
		cJSON_Delete(pJson);
	}
	return statuesCode;
}
/**
 * 收到请求
 * @param sender - 请求者名称，以模块名称命名
 * @param request - 请求内容
 * @param response - 请求回复内容
 * @return 200-成功；其它-错误码，参考http错误码
 */
int DtdMngService::onRequest(const std::string &sender, const std::string &request, std::string &response)
{
	logger(LEVEL_DEBUG, "sender:%s, request:%s\n", sender.c_str(), request.c_str());
	cJSON* pJson = cJSON_Parse(request.c_str());
	if (pJson == NULL){
		return makeResponse(NULL, response, STATUS_BAD_RREQUEST, ERR_INVALID_JSON, "Invalid JSON format");
	}
	cJSON *pMethod = cJSON_GetObjectItem(pJson, "Method");
	cJSON* pParam = cJSON_GetObjectItem(pJson, "Param");
	if (pMethod == NULL){
		return makeResponse(pJson, response, STATUS_BAD_RREQUEST, ERR_NO_METHOD_REQUEST, "No Method in request");
	}
	if (pParam == NULL){
		return makeResponse(pJson, response, STATUS_BAD_RREQUEST, ERR_NO_PARAM_REQUEST, "No Param in request");
	}
	string method = pMethod->valuestring;
	int errorCode = 0;
	string errorMsg;
	if (method == "Object.Update"){
		MsgObjectUpdate msg;
		msg.enabled = true;
		Helper::parseMsgObjectUpdateFromJson(pParam, msg);
		errorCode = pManager->onObjectUpdateRequest(msg, errorMsg);
	}else if (method == "Object.Delete"){
		MsgObjectDelete msg;
		Helper::parseMsgObjectDeleteFromJson(pParam, msg);
		errorCode = pManager->onObjectDeleteRequest(msg, errorMsg);
	}else{
		return makeResponse(pJson, response, STATUS_METHOD_NOT_ALLOW, ERR_METHOD_NOT_ALLOW, "Method not allow");
	}

	if (errorCode != 0){
		return makeResponse(pJson, response, STATUS_BAD_RREQUEST, errorCode, errorMsg.c_str());
	}else{
		return makeResponse(pJson, response, SUCCESS, ERR_NO_ERROR, "success");
	}
}

void DtdMngService::stop() {
}

void DtdMngService::start() throw (DeltaDoreException){
	list<MsgObjectUpdate*> deviceList;
	pManager->getAllDeviceInfo(deviceList);
	if (deviceList.size() < 1){
		return;
	}
	string key = "SOFI_HCC";
	string body;
	string response;
	Helper::listMsgObjectUpdateToJson(deviceList, pManager->config.moduleId, body);
	sf::ExtServicesManager::instance()->request(key, pManager->config.moduleId, body, response);
	logger(LEVEL_INFO, "Request ObjectList.Reg result is %s\n", response.c_str());

}
