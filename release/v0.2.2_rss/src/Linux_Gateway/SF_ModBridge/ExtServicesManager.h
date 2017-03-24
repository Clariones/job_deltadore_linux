//
//  ExtServicesManager.h
//  SF_ModBridge
//
//  Created by hu_danyuan on 16/12/02.
//  Copyright © 2016年 杭州速凡网络科技有限公司. All rights reserved.
//
//  扩展服务集

#ifndef HomeControlCenter_ExtServicesManager_h
#define HomeControlCenter_ExtServicesManager_h

#include "IExtService.h"

namespace sf
{
    class ExtServicesManager
    {
    public:
        /** 单例 */
        static ExtServicesManager* instance();
        /**
         * 注册服务
         * @param key - 服务id
         * @param server - 服务提供者
         */
        void registerService(const std::string &key, IExtService *service);
        /**
         * 取消服务
         * @param server - 服务提供者
         */
        void unregisterService(IExtService *service);

        /**
         * 请求服务
         * @param key - 服务id
         * @param sender - 请求者名称，以模块名称命名
         * @param body - 请求内容，具体参考协议文档
         * @param response - 请求回复内容
         * @return 200-成功；其它-错误码，参考http错误码
         */
        int request(const std::string &key, const std::string &sender, const std::string &body, std::string &response);
    };
}

#endif /* HomeControlCenter_ExtServicesManager_h */
