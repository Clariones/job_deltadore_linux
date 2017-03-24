//
//  SubServicesManager.h
//  SF_ModBridge
//
//  Created by hu_danyuan on 16/12/02.
//  Copyright © 2016年 杭州速凡网络科技有限公司. All rights reserved.
//
//  订阅服务

#ifndef HomeControlCenter_SubServicesManager_h
#define HomeControlCenter_SubServicesManager_h

#include "ISubscribeListener.h"

namespace sf
{
    /************************* 订阅服务定义 *************************/
    /** 设备状态通知 */
    extern const std::string kSubscribeServiceObjectState;
    /** 执行命令通知 */
    extern const std::string kSubscribeServiceActionExecute;
    /** 执行规则通知 */
    extern const std::string kSubscribeServiceRuleExecute;
    /** 系统布撤防状态通知 */
    extern const std::string kSubscribeServiceSystemGuard;
    /** 系统紧急报警通知 */
    extern const std::string kSubscribeServiceSystemEmergency;
    /** 系统配置通知 */
    extern const std::string kSubscribeServiceSystemConfig;
    
    class SubServicesManager
    {
    public:
        /** 单例 */
        static SubServicesManager* instance();
        /**
         * 订阅指定的服务
         * @param key - 订阅服务id，订阅的服务有消息时会立刻通知订阅者
         * @param listener - 订阅消息监听
         */
        void subscribe(const std::string &key, ISubscribeListener *listener);
        /**
         * 取消指定订阅者的指定服务
         * @param key - 订阅服务id，订阅的服务有消息时会立刻通知订阅者
         * @param listener - 订阅消息监听
         */
        void unsubscribe(const std::string &key, ISubscribeListener *listener);
        /**
         * 取消listener所有订阅服务
         * @param listener - 订阅消息监听
         */
        void unsubscribe(ISubscribeListener *listener);
        /**
         * 发布消息到指定服务
         * @param key - 订阅服务id，订阅的服务有消息时会立刻通知订阅者
         * @param message - 发布的消息
         */
        void publishMessage(const std::string &publisher, const std::string &key, const std::string &message);
        /**
         * 发布数据到指定服务
         * @param key - 订阅服务id，订阅的服务有消息时会立刻通知订阅者
         * @param buf - 发布的数据
         * @param size - 发布的数据大小
         */
        void publishData(const std::string &publisher, const std::string &key, char *buf, unsigned int size);
    };
}

#endif /* HomeControlCenter_SubServerManager_h */
