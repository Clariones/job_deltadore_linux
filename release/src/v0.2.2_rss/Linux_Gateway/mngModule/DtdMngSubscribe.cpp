//
//  Subscribe.cpp
//  ModuleSample
//
//  Created by hu_danyuan on 16/12/02.
//  Copyright © 2016年 杭州速凡网络科技有限公司. All rights reserved.
//

#include "DtdMngSubscribe.h"
#include "utils/fileUtils.h"
#include "SF_ModBridge/SubServicesManager.h"
#include <stdio.h>

using namespace cla;
/**
 * 收到订阅消息
 * @param publisher - 发布者唯一标示
 * @param key - 订阅服务id，订阅的服务有消息时会立刻通知订阅者
 * @param message - 发布的消息
 */
void DtdMngSubscribe::onSubscriptionMessage(const std::string &publisher,
		const std::string &key, const std::string &message) {
	const char* pMsgString = message.c_str();
	logger(LEVEL_DEBUG, "<onSubscriptionMessage>publisher:%s (%s) %s\n", publisher.c_str(),
			key.c_str(), pMsgString);
	cJSON* pJson = NULL;
	try {
		if (key == sf::kSubscribeServiceObjectState){
			// TODO
			return;
		}else if (key == sf::kSubscribeServiceActionExecute){
			MsgActionExecute* pMsg = new MsgActionExecute;
			pJson = cJSON_Parse(message.c_str());
			Helper::parseMsgActionExecute(pJson, *pMsg);
			pManager->onSetValueAction(pMsg);
		}
	}catch (DeltaDoreException e){
		printf("Error Code=%d", e.getCode());
	}
	if (pJson != NULL){
		cJSON_Delete(pJson);
	}
}
/**
 * 收到订阅数据
 * @param publisher - 发布者唯一标示
 * @param key - 订阅服务id，订阅的服务有消息时会立刻通知订阅者
 * @param buf - 发布的数据
 * @param size - 发布的数据大小
 */
void DtdMngSubscribe::onSubscriptionData(const std::string &publisher,
		const std::string &key, char *buf, unsigned int size) {
	printf("<onSubscriptionData>publisher:%s (%s) %d\n", publisher.c_str(),
			key.c_str(), size);
}

void cla::DtdMngSubscribe::stop() {
}
