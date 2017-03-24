//
//  ISubscribeListener.h
//  SF_ModBridge
//
//  Created by hu_danyuan on 16/12/02.
//  Copyright © 2016年 杭州速凡网络科技有限公司. All rights reserved.
//
//  订阅服务listener

#ifndef HomeControlCenter_ISubscribeListener_h
#define HomeControlCenter_ISubscribeListener_h

#include <string>

namespace sf
{
	/** 回调接口中不要做耗时操作 */
	class ISubscribeListener
	{
	public:
	    /**
	     * 收到订阅消息
	     * @param publisher - 发布者唯一标示
	     * @param key - 订阅服务id，订阅的服务有消息时会立刻通知订阅者
	     * @param message - 发布的消息
	     */
	    virtual void onSubscriptionMessage(const std::string &publisher, const std::string &key, const std::string &message) = 0;
	    /**
	     * 收到订阅数据
	     * @param publisher - 发布者唯一标示
	     * @param key - 订阅服务id，订阅的服务有消息时会立刻通知订阅者
	     * @param buf - 发布的数据
	     * @param size - 发布的数据大小
	     */
	    virtual void onSubscriptionData(const std::string &publisher, const std::string &key, char *buf, unsigned int size) = 0;
	};
}

#endif /* HomeControlCenter_ISubscribeListener_h */
