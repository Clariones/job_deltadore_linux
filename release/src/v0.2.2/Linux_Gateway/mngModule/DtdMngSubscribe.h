//
//  Subscribe.h
//  ModuleSample
//
//  Created by hu_danyuan on 16/12/02.
//  Copyright © 2016年 杭州速凡网络科技有限公司. All rights reserved.
//
//  订阅服务

#ifndef __DTD_MNG_SUBSCRIBE_H__
#define __DTD_MNG_SUBSCRIBE_H__

#include "SF_ModBridge/ISubscribeListener.h"
#include "MangementModule.h"


namespace cla {

class DtdMngSubscribe: public sf::ISubscribeListener {
public:
	DtdMngSubscribe() :
			pManager(NULL) {
	}
	virtual ~DtdMngSubscribe() {
	}
public:
	/**
	 * 收到订阅消息
	 * @param publisher - 发布者唯一标示
	 * @param key - 订阅服务id，订阅的服务有消息时会立刻通知订阅者
	 * @param message - 发布的消息
	 */
	virtual void onSubscriptionMessage(const std::string &publisher,
			const std::string &key, const std::string &message);
	/**
	 * 收到订阅数据
	 * @param publisher - 发布者唯一标示
	 * @param key - 订阅服务id，订阅的服务有消息时会立刻通知订阅者
	 * @param buf - 发布的数据
	 * @param size - 发布的数据大小
	 */
	virtual void onSubscriptionData(const std::string &publisher,
			const std::string &key, char *buf, unsigned int size);

	void stop();
	void start() throw (DeltaDoreException) {};

	const MangementModule* getManager() const {
		return pManager;
	}

	void setManager(MangementModule* manager) {
		pManager = manager;
	}

protected:
	MangementModule* pManager = NULL;
};
}

#endif /* HomeControlCenter_Subscribe_h */
