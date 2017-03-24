#include "SF_ModBridge/SubServicesManager.h"
#include <iostream>
using namespace std;

namespace sf {
/** 设备状态通知 */
const string kSubscribeServiceObjectState = "/object/state";
/** 执行命令通知 */
const string kSubscribeServiceActionExecute = "/action/execute";
/** 执行规则通知 */
const string kSubscribeServiceRuleExecute = "run_rule";
/** 系统布撤防状态通知 */
const string kSubscribeServiceSystemGuard = "notify";
/** 系统紧急报警通知 */
const string kSubscribeServiceSystemEmergency = "alert";
/** 系统配置通知 */
const string kSubscribeServiceSystemConfig = "config";

SubServicesManager* theInstance = 0;

SubServicesManager* SubServicesManager::instance() {
	cout<< "[TEST_STUB]" << "get SubServicesManager Instance" << endl;
	return theInstance;
}

/**
 * 订阅指定的服务
 * @param key - 订阅服务id，订阅的服务有消息时会立刻通知订阅者
 * @param listener - 订阅消息监听
 */
void SubServicesManager::subscribe(const std::string &key,
		ISubscribeListener *listener) {
	cout << "[TEST_STUB]" << this << ":subscribe " << key << endl;
}
/**
 * 取消指定订阅者的指定服务
 * @param key - 订阅服务id，订阅的服务有消息时会立刻通知订阅者
 * @param listener - 订阅消息监听
 */
void SubServicesManager::unsubscribe(const std::string &key,
		ISubscribeListener *listener) {

}
/**
 * 取消listener所有订阅服务
 * @param listener - 订阅消息监听
 */
void SubServicesManager::unsubscribe(ISubscribeListener *listener) {

}
/**
 * 发布消息到指定服务
 * @param key - 订阅服务id，订阅的服务有消息时会立刻通知订阅者
 * @param message - 发布的消息
 */
void SubServicesManager::publishMessage(const std::string &publisher,
		const std::string &key, const std::string &message) {
	cout << "[[[publishMessage]]] from " << publisher <<", key=" << key << ", message=" << message << endl;
}
/**
 * 发布数据到指定服务
 * @param key - 订阅服务id，订阅的服务有消息时会立刻通知订阅者
 * @param buf - 发布的数据
 * @param size - 发布的数据大小
 */
void SubServicesManager::publishData(const std::string &publisher,
		const std::string &key, char *buf, unsigned int size) {

}
}
