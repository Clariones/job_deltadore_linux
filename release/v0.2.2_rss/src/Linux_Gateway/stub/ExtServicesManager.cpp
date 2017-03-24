#include "SF_ModBridge/ExtServicesManager.h"
#include <iostream>
using namespace std;

namespace sf
{
static ExtServicesManager* theInstance = NULL;
        /** 单例 */
        ExtServicesManager* ExtServicesManager::instance(){
            if (theInstance == NULL){
                theInstance = new ExtServicesManager();
            }
        	return theInstance;
        }
        /**
         * 注册服务
         * @param key - 服务id
         * @param server - 服务提供者
         */
        void ExtServicesManager::registerService(const std::string &key, IExtService *service){
        }
        /**
         * 取消服务
         * @param server - 服务提供者
         */
        void ExtServicesManager::unregisterService(IExtService *service){
        }

        /**
         * 请求服务
         * @param key - 服务id
         * @param sender - 请求者名称，以模块名称命名
         * @param body - 请求内容，具体参考协议文档
         * @param response - 请求回复内容
         * @return 200-成功；其它-错误码，参考http错误码
         */
        int ExtServicesManager::request(const std::string &key, const std::string &sender, const std::string &body, std::string &response){
        	cout << "request to " << key << " by " << sender << ": " << body << endl;
        	return 0;
        }

}
