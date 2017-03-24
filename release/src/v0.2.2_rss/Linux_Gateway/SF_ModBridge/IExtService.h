//
//  IExtService.h
//  SF_ModBridge
//
//  Created by hu_danyuan on 16/12/02.
//  Copyright © 2016年 杭州速凡网络科技有限公司. All rights reserved.
//
//  扩展服务接口定义

#ifndef HomeControlCenter_IExtService_h
#define HomeControlCenter_IExtService_h

#include <string>

namespace sf
{
	/** 回调接口中不要做耗时操作 */
	class IExtService
	{
	public:
	    /**
	     * 收到请求
	     * @param sender - 请求者名称，以模块名称命名
	     * @param request - 请求内容
	     * @param response - 请求回复内容
	     * @return 200-成功；其它-错误码，参考http错误码
	     */
	    virtual int onRequest(const std::string &sender, const std::string &request, std::string &response) = 0;
	};
}

#endif /* HomeControlCenter_IExtService_h */
