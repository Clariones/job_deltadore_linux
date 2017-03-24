#ifndef __HELPER_H__
#define __HELPER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <list>
#include <pthread.h>

#include "DeltaDoreException.h"
#include "utils/cJSON.h"
#include "utils/fileUtils.h"
#include "MsgActionExecute.h"

#define DEFAULT_MODULE_ID "DeltaDoreDriver"
#define DEFAULT_MANUFACTURE "Delta Dore"
#define DEFAULT_DEVICE_NAME "/dev/ttyUSB0"
#define EXIT_WITH_ERROR(code) throw DeltaDoreException(code)

#define CONST_MSG_FIELD_NOT_SET "<!--not-set!-->"

using namespace std;

typedef unsigned int TIME_TS;

typedef struct {
	string moduleId;
	string deviceName;
	string manufacturer;
	int webCommandPort;
} ModuleConfig;

typedef struct {
	string id;
	string type;
	string subtype;
	bool enabled;
	string name;
	string cluster;
	string region;
	string icon;
	string manufacturer;
	string model;
	string value;
	int network;
	int node;
	int deviceType;
	bool simpleOnOff;
	int refreshResult;
	int level;
	int position;
	int color;
	float temperature;
	bool onLine;
	TIME_TS mustReportStateTs;
} MsgObjectUpdate;
typedef MsgObjectUpdate DeviceState;

typedef struct {
	int network;
	int node;
	int deviceType;
	int level;
	int position;
	bool simpleOnOff;
	bool multiColor;
	int channelCount;
	float temperature;
	int redColor;
	int greenColor;
	int blueColort;
	TIME_TS nextRefreshTs;
	string name;
	string region;
	bool online;
} DevicePhysicalState;



typedef struct {
	string id;
} MsgObjectDelete;

#define STATUS_BAD_RREQUEST 400
#define STATUS_METHOD_NOT_ALLOW 405

#define SUCCESS 200

#define ACTION_TYPE_SetValue  0
#define ACTION_TYPE_toggleValue 1
#define ACTION_TYPE_refreshState 2
#define ACTION_TYPE_cycleOnOff 3
#define ACTION_TYPE_delayOff 4
#define ACTION_TYPE_setEnable 5
#define ACTION_TYPE_queryObjects 6
#define ACTION_TYPE_Repeat 7
#define ACTION_TYPE_SetAllValue  8

#define REFRESH_TYPE_DELETED 1
#define REFRESH_TYPE_NEW 2
#define REFRESH_TYPE_UPDATED 3
#define REFRESH_TYPE_NOCHANGE 4
#define REFRESH_TYPE_SET_BY_CMD 5
#define REFRESH_TYPE_OFFLINE 6
#define REFRESH_TYPE_ONLINE 7
#define REFRESH_TYPE_INIT 0

#define CONST_TYPE_DIMMER  "DimmableLight"
#define CONST_TYPE_LIGHT  "OnOffLight"
#define CONST_TYPE_COLORLIGHT  "ColorDimmerSwitch"
#define CONST_TYPE_SHADE  "Shade"
#define CONST_TYPE_TEMP_SENSOR "TempHumCollector"

#define CONST_CLUSTER_ONOFF "6"
#define CONST_CLUSTER_LEVEL "8"
#define CONST_CLUSTER_WARN  "0x0502"

enum DEVICE_TYPE {
	DEVICE_UNKNOWN, DEVICE_LIGHT, DEVICE_ROLLER_SHUTTER, DEVICE_TEMPERATURE_SENSOR
};

enum COMMAND_TYPE {
	CMD_SET_LIGHT_ON,
	CMD_SET_LIGHT_OFF,
	CMD_SET_LIGHT_LEVEL,
	CMD_SET_LIGHT_TOGGLE,
	CMD_GET_LIGHT_INFO,
	CMD_GET_LIGHT_STATE,

	CMD_SET_ROLLERSHUTTER_OPEN,
	CMD_SET_ROLLERSHUTTER_CLOSE,
	CMD_SET_ROLLERSHUTTER_STOP,
	CMD_GET_ROLLERSHUTTER_INFO,
	CMD_GET_ROLLERSHUTTER_STATE,

	CMD_GET_TEMPERATURE_SENSOR_STATE,

	CMD_REFRESH_DEVICE_STATE,
	CMD_SET_DEVICE_NAME,

};

class Helper {
public:
	static void parseConfigFromJson(const char* jsonStr, ModuleConfig& config);
	static void parseDeviceConfigFromJson(const char* jsonStr, list<DeviceState*>& deviceList);
	static int lastIndexOf(const char* string, const char* find);
	static void getNetworkAndNodeFromId(const char* id, int& network, int& node);
	static char* getIdByDeviceInfo(const char* moduleId, int network, int node);

	static void parseMsgObjectUpdateFromJson(cJSON* pJson, MsgObjectUpdate& msg) throw (DeltaDoreException);
	static void parseMsgObjectDeleteFromJson(cJSON* pJson, MsgObjectDelete& msg) throw (DeltaDoreException);
	static void parseMsgActionExecute(cJSON* pJson, MsgActionExecute& msg)throw (DeltaDoreException);
	static void parseSimpleMsgActionExecute(cJSON* pJson, MsgActionExecute& msg)throw (DeltaDoreException);
	static void listMsgObjectUpdateToJson(list<MsgObjectUpdate*>& deviceList, const string& serverName, string& jsonStr);
	static void toJsonRequestObjectReg(MsgObjectUpdate* pDevice, const string& serverName, string& jsonStr);
	static void toJsonMsgObjectState(MsgObjectUpdate& msg, string& jsonStr);
	static void toJsonReqObjectUnReg(string& id, string& jsonStr);
	static cJSON* toJsonDeviceDatas(list<MsgObjectUpdate*>& deviceList);
	static TIME_TS getCurrentSystemMS();
	static bool timeIsUp(TIME_TS curTs, TIME_TS plannedTs);
	static void sleep_ms(TIME_TS mSec);
	static void toJsonFakeMsgObjectState(MsgObjectUpdate& msg, const char* value, string& jsonStr);
};
#endif
