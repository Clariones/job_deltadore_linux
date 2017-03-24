//
//  BaseModule.h
//  SF_ModBridge
//
//  Created by hu_danyuan on 16/12/07.
//  Copyright © 2016年 杭州速凡网络科技有限公司. All rights reserved.
//
//  模块类

#ifndef sf_BaseModule_h
#define sf_BaseModule_h

#include <string>

namespace sf
{
    typedef struct BaseModule
    {
    public:
        // 模块加载接口，返回模块信息
        typedef sf::BaseModule* (*FUNC_LOAD)();
        // 初始化接口，模块的初始化工作在这里进行
        typedef int  (*FUNC_INIT)(unsigned char);
        // 模块反初始化接口，模块卸载之前需要释放的资源等操作在这里进行
        typedef void (*FUNC_DEINIT)();
        // 模块启动接口，把模块中线程的启动，服务的启动等复杂的逻辑放在这里进行
        typedef int  (*FUNC_START)();
        // 模块停止接口，与FUNC_START方法对应
        typedef void (*FUNC_STOP)();
        
        // 模块许可密钥
        std::string secret;
        // 模块名称，不超过20字符
        std::string name;
        // 模块版本号，如1.0.0
        std::string version;
        FUNC_INIT init;
        FUNC_DEINIT deinit;
        FUNC_START start;
        FUNC_STOP stop;
    }BaseModule;
}
#endif /* sf_BaseModule_h */
