//
//  ExtService.h
//  ModuleSample
//
//  Created by hu_danyuan on 16/12/02.
//  Copyright © 2016年 杭州速凡网络科技有限公司. All rights reserved.
//
//  订阅服务

#ifndef __DTD_MNG_SERVICE_H__
#define __DTD_MNG_SERVICE_H__

#include "SF_ModBridge/IExtService.h"
#include "Helper.h"
#include "utils/fileUtils.h"
#include "MangementModule.h"

namespace cla {

class DtdMngService: public sf::IExtService {
public:
	DtdMngService() :
			pManager(NULL) {
	}
	virtual ~DtdMngService() {
	}
public:
	/**
	 * 收到请求
	 * @param sender - 请求者名称，以模块名称命名
	 * @param request - 请求内容
	 * @param response - 请求回复内容
	 * @return 200-成功；其它-错误码，参考http错误码
	 */
	virtual int onRequest(const std::string &sender, const std::string &request,
			std::string &response);

	void stop();
	void start() throw (DeltaDoreException);
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

#endif /* HomeControlCenter_ExtService_h */
