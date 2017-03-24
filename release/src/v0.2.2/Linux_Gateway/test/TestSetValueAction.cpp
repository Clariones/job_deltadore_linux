#include <assert.h>

#include "TestTool.h"
using namespace std;

void whenTestEnd() {
	printf("=============================================================\n");
}
void case_001() {
	string key = "/action/execute";
	string message = loadJson("setValue01.json");
	test_onMessage(key, message);

	// 首先，应该收到actionMessageList这个队列里
	assert(pManager->actionMessageList.size() == 1);

	// 然后这个命令就被送到deviceCommand 里
	pManager->actionMessageProcessing();
	assert(pManager->actionMessageList.size() == 0);
	assert(pManager->deviceCommandList.size() == 1);
//	DeviceActionCommand* pCmd = pManager->deviceCommandList.front();
//	assert(pCmd->commandCode == CMD_SET_LIGHT_ON);
	printf("\ncase_001: should call setLightOn 0,1\n");
	pManager->processDeviceCommands();
	assert(pManager->deviceCommandList.size() == 0);
	whenTestEnd();
}
void case_002() {
	// 做一次轮询
	printf(
			"\ncase_002: do polling again. Because this is no real time-based device, so should query all devices again\n");
	pManager->pollingRoutine();
	whenTestEnd();
}
void case_003() {
	// 模拟设备读取命令，也就是给所有的数据时间都重置为1分钟后
	TEST_DEVICE_TS = Helper::getCurrentSystemMS() + 60 * 1000;
	printf("\ncase_003: do polling again. Because set time to 1min later, so should not query any device\n");
	pManager->pollingRoutine();
	whenTestEnd();
}

void case_004() {
	// enable device 0，1， 然后再设置开关level各一次
	string key = "/action/execute";
	string message1 = loadJson("enable01.json");
	string message2 = loadJson("setValue01.json");
	string message3 = loadJson("off01.json");
	string message4 = loadJson("level01.json");
	test_onMessage(key, message1);

	printf("\nCase_004: Just enable 0,1, should not call any device\n");
	assert(pManager->actionMessageList.size() == 1);
	pManager->actionMessageProcessing();
	assert(pManager->actionMessageList.size() == 0);
	pManager->processDeviceCommands();
	assert(pManager->deviceCommandList.size() == 0);

	test_onMessage(key, message2);
	printf("Case_004: should call setLightOn 0,1\n");
	assert(pManager->actionMessageList.size() == 1);
	pManager->actionMessageProcessing();
	assert(pManager->actionMessageList.size() == 0);
	assert(pManager->deviceCommandList.size() == 1);
	pManager->processDeviceCommands();
	assert(pManager->deviceCommandList.size() == 0);

	test_onMessage(key, message3);
	test_onMessage(key, message4);
	assert(pManager->actionMessageList.size() == 2);
	pManager->actionMessageProcessing();
	assert(pManager->actionMessageList.size() == 0);
	assert(pManager->deviceCommandList.size() == 2);
	printf("Case_004: Should have call switchOffLight(0,1) &  call setLightLevel(0,1,100)\n");
	pManager->processDeviceCommands();
	assert(pManager->deviceCommandList.size() == 0);
	whenTestEnd();
}

void case_005() {
	// 测试两个窗帘的开关停命令
	string key = "/action/execute";
	string message1 = loadJson("open13.json");
	string message2 = loadJson("stop13.json");
	string message3 = loadJson("close14.json");
	test_onMessage(key, message1);
	test_onMessage(key, message2);
	test_onMessage(key, message3);
	assert(pManager->actionMessageList.size() == 3);
	pManager->actionMessageProcessing();
	assert(pManager->actionMessageList.size() == 0);
	assert(pManager->deviceCommandList.size() == 3);
	printf("\nCase_005: Should have call openRollerShutter(1,3), stopRollerShutter(1,3) & closeRollerShutter(1,4)\n");
	pManager->processDeviceCommands();
	assert(pManager->deviceCommandList.size() == 0);
	whenTestEnd();
}
void case_006() {
	// 测试toggle 02, 13
	string key = "/action/execute";
	string message1 = loadJson("toggle02.json");
	string message2 = loadJson("toggle13.json");
	test_onMessage(key, message1);
	test_onMessage(key, message2);
	assert(pManager->actionMessageList.size() == 2);
	pManager->actionMessageProcessing();
	assert(pManager->actionMessageList.size() == 0);
	assert(pManager->deviceCommandList.size() == 1);
	printf("\nCase_006: Should have call toggleLight(0,2)\n");
	pManager->processDeviceCommands();
	assert(pManager->deviceCommandList.size() == 0);
	whenTestEnd();
}
void case_007() {
	// 测试send-read-request
	string key = "/action/execute";
	string message1 = loadJson("refresh14.json");
	test_onMessage(key, message1);
	assert(pManager->actionMessageList.size() == 1);
	pManager->actionMessageProcessing();
	assert(pManager->actionMessageList.size() == 0);
	assert(pManager->deviceCommandList.size() == 1);
	printf("\nCase_007: Should have call queryRoolershutterPhyState(1,4)\n");
	pManager->processDeviceCommands();
	assert(pManager->deviceCommandList.size() == 0);
	whenTestEnd();
}
void case_008() {
	// enable device 0，1， 然后再设置开关level各一次
	string key = "/action/execute";
	string message1 = loadJson("disable02.json");
	string message2 = loadJson("toggle02.json");
	test_onMessage(key, message1);
	test_onMessage(key, message2);

	assert(pManager->actionMessageList.size() == 2);
	pManager->actionMessageProcessing();
	assert(pManager->actionMessageList.size() == 0);
	assert(pManager->deviceCommandList.size() == 0);
	pManager->processDeviceCommands();
	assert(pManager->deviceCommandList.size() == 0);
	printf("\nCase_008: Should NOT call any device function\n");
	whenTestEnd();
}
void case_009() {
	// 测试窗帘的cycle onoff
	string key = "/action/execute";
	string message1 = loadJson("cycleOnOff13.json");
	test_onMessage(key, message1);
	printf("\n");
	TIME_TS t1 = Helper::getCurrentSystemMS();
	TIME_TS t2;
	unsigned int actionBefore[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 };
	unsigned int actionAfter[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 };
	unsigned int deviceBefore[] = { 2, 2, 2, 2, 1, 1, 3, 2, 2, 2, 1, 1, 3, 2, 2, 2, 1, 1, 1 };
	unsigned int deviceAfter[] = { 2, 2, 2, 1, 1, 1, 2, 2, 2, 1, 1, 1, 2, 2, 2, 1, 1, 1, 0 };
	int commandCalled[] = { 0, 0, 0, 1, 0, 0, 2, 0, 0, 1, 0, 0, 2, 0, 0, 1, 0, 0, 2 };
	for (int i = 0; i <= 18; i++) {
		t2 = Helper::getCurrentSystemMS();
		TIME_TS passedTime = t2 - t1;

		switch (commandCalled[i]) {
		case 0: {
			printf("case_009: %5.3fsec Should not call any device command\n", passedTime / 1000.0);
			break;
		}
		case 1: {
			printf("case_009: %5.3fsec Should call open\n", passedTime / 1000.0);
			break;
		}
		case 2: {
			printf("case_009: %5.3fsec Should call close\n", passedTime / 1000.0);
			break;
		}
		}
		assert(pManager->actionMessageList.size() == actionBefore[i]);
		pManager->actionMessageProcessing();
		assert(pManager->actionMessageList.size() == actionAfter[i]);
		assert(pManager->deviceCommandList.size() == deviceBefore[i]);
		pManager->processDeviceCommands();
		assert(pManager->deviceCommandList.size() == deviceAfter[i]);
		Helper::sleep_ms(1000);
	}
	whenTestEnd();
}
void case_010() {
	// 测试light的cycle onoff
	string key = "/action/execute";
	string message1 = loadJson("cycleOnOff01.json");
	test_onMessage(key, message1);
	printf("\n");
	TIME_TS t1 = Helper::getCurrentSystemMS();
	TIME_TS t2;
	unsigned int actionBefore[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 };
	unsigned int actionAfter[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 };
	unsigned int deviceBefore[] = { 2, 2, 2, 2, 1, 1, 3, 2, 2, 2, 1, 1, 3, 2, 2, 2, 1, 1, 1 };
	unsigned int deviceAfter[] = { 2, 2, 2, 1, 1, 1, 2, 2, 2, 1, 1, 1, 2, 2, 2, 1, 1, 1, 0 };
	int commandCalled[] = { 0, 0, 0, 1, 0, 0, 2, 0, 0, 1, 0, 0, 2, 0, 0, 1, 0, 0, 2 };
	for (int i = 0; i <= 18; i++) {
		t2 = Helper::getCurrentSystemMS();
		TIME_TS passedTime = t2 - t1;

		switch (commandCalled[i]) {
		case 0: {
			printf("case_010: %5.3fsec Should not call any device command\n", passedTime / 1000.0);
			break;
		}
		case 1: {
			printf("case_010: %5.3fsec Should call setLightOn\n", passedTime / 1000.0);
			break;
		}
		case 2: {
			printf("case_010: %5.3fsec Should call setLightOff\n", passedTime / 1000.0);
			break;
		}
		}
		assert(pManager->actionMessageList.size() == actionBefore[i]);
		pManager->actionMessageProcessing();
		assert(pManager->actionMessageList.size() == actionAfter[i]);
		assert(pManager->deviceCommandList.size() == deviceBefore[i]);
		pManager->processDeviceCommands();
		assert(pManager->deviceCommandList.size() == deviceAfter[i]);
		Helper::sleep_ms(1000);
	}
	whenTestEnd();
}
void case_011() {
	// 测试 repeat
	string key = "/action/execute";
	string message1 = loadJson("repeatCase1.json");
	test_onMessage(key, message1);
	printf("\n");
	TIME_TS t1 = Helper::getCurrentSystemMS();
	TIME_TS t2;
	unsigned int actionBefore[] = { 1, 1, 1, 0, 0 };
	unsigned int actionAfter[] = { 1, 1, 0, 0, 0 };
	unsigned int deviceBefore[] = { 2, 0, 2, 0, 0 };
	unsigned int deviceAfter[] = { 0, 0, 0, 0, 0 };
	int commandCalled[] = { 1, 0, 1, 0, 0 };
	for (int i = 0; i < 5; i++) {
		t2 = Helper::getCurrentSystemMS();
		TIME_TS passedTime = t2 - t1;

		switch (commandCalled[i]) {
		case 0: {
			printf("case_011: %5.3fsec Should not call any device command\n", passedTime / 1000.0);
			break;
		}
		case 1: {
			printf("case_011: %5.3fsec Should call toggleLight and close roller-shutter\n", passedTime / 1000.0);
			break;
		}
		}
		assert(pManager->actionMessageList.size() == actionBefore[i]);
		pManager->actionMessageProcessing();
		assert(pManager->actionMessageList.size() == actionAfter[i]);
		assert(pManager->deviceCommandList.size() == deviceBefore[i]);
		pManager->processDeviceCommands();
		assert(pManager->deviceCommandList.size() == deviceAfter[i]);
		Helper::sleep_ms(1000);
	}
	whenTestEnd();
}
void case_012() {
	// 测试 delay off
	// 测试 repeat
	string key = "/action/execute";
	string message1 = loadJson("delayOff01.json");
	test_onMessage(key, message1);
	printf("\n");
	TIME_TS t1 = Helper::getCurrentSystemMS();
	TIME_TS t2;
	unsigned int actionBefore[] = { 1, 0, 0, 0, 0, 0 };
	unsigned int actionAfter[] = { 0, 0, 0, 0, 0, 0 };
	unsigned int deviceBefore[] = { 1, 1, 1, 1, 1, 1 };
	unsigned int deviceAfter[] = { 1, 1, 1, 1, 1, 0 };
	int commandCalled[] = { 0, 0, 0, 0, 0, 1 };
	for (int i = 0; i < 6; i++) {
		t2 = Helper::getCurrentSystemMS();
		TIME_TS passedTime = t2 - t1;

		switch (commandCalled[i]) {
		case 0: {
			printf("case_012: %5.3fsec Should not call any device command\n", passedTime / 1000.0);
			break;
		}
		case 1: {
			printf("case_012: %5.3fsec Should call setLightOff\n", passedTime / 1000.0);
			break;
		}
		}
		assert(pManager->actionMessageList.size() == actionBefore[i]);
		pManager->actionMessageProcessing();
		assert(pManager->actionMessageList.size() == actionAfter[i]);
		assert(pManager->deviceCommandList.size() == deviceBefore[i]);
		pManager->processDeviceCommands();
		assert(pManager->deviceCommandList.size() == deviceAfter[i]);
		Helper::sleep_ms(1000);
	}
	whenTestEnd();
}
void case_013() {
	// 测试 	query objects
	string key = "/action/execute";
	string message = loadJson("queryObjects.json");
	test_onMessage(key, message);

	// 首先，应该收到actionMessageList这个队列里
	assert(pManager->actionMessageList.size() == 1);

	// 然后这个命令就被送到deviceCommand 里
	pManager->actionMessageProcessing();
	assert(pManager->actionMessageList.size() == 0);
	printf("\ncase_013: You should see all device was published\n");
	// 命令直接处理，不进入device command
	assert(pManager->deviceCommandList.size() == 0);
	pManager->processDeviceCommands();
	assert(pManager->deviceCommandList.size() == 0);
	whenTestEnd();
}
void case_014() {
		string key = "/action/execute";
		string message = loadJson("setAllValue.json");
		test_onMessage(key, message);

		// 首先，应该收到actionMessageList这个队列里
		assert(pManager->actionMessageList.size() == 1);

		// 然后这个命令就被送到deviceCommand 里
		pManager->actionMessageProcessing();
		assert(pManager->actionMessageList.size() == 0);
		assert(pManager->deviceCommandList.size() == 1);
	//	DeviceActionCommand* pCmd = pManager->deviceCommandList.front();
	//	assert(pCmd->commandCode == CMD_SET_LIGHT_ON);
		printf("\ncase_014: should call setLightOn 0,2\n");
		pManager->processDeviceCommands();
		assert(pManager->deviceCommandList.size() == 0);
		whenTestEnd();
}
void case_015() {
		string key = "/action/execute";
		string message = loadJson("setAllLevel.json");
		test_onMessage(key, message);

		// 首先，应该收到actionMessageList这个队列里
		assert(pManager->actionMessageList.size() == 1);

		// 然后这个命令就被送到deviceCommand 里
		pManager->actionMessageProcessing();
		assert(pManager->actionMessageList.size() == 0);
		assert(pManager->deviceCommandList.size() == 3);
	//	DeviceActionCommand* pCmd = pManager->deviceCommandList.front();
	//	assert(pCmd->commandCode == CMD_SET_LIGHT_ON);
		printf("\ncase_014: should call 3 devices\n");
		pManager->processDeviceCommands();
		assert(pManager->deviceCommandList.size() == 0);
		whenTestEnd();
}
void case_016() {
	string key = "/action/execute";
	string message = loadJson("up_00.json");
	test_onMessage(key, message);

	// 首先，应该收到actionMessageList这个队列里
	assert(pManager->actionMessageList.size() == 1);

	// 然后这个命令就被送到deviceCommand 里
	pManager->actionMessageProcessing();
	assert(pManager->actionMessageList.size() == 0);
	assert(pManager->deviceCommandList.size() == 1);
//	DeviceActionCommand* pCmd = pManager->deviceCommandList.front();
//	assert(pCmd->commandCode == CMD_SET_LIGHT_ON);
	printf("\ncase_001: should call setLightOn 0,1\n");
	pManager->processDeviceCommands();
	assert(pManager->deviceCommandList.size() == 0);
	whenTestEnd();
}
void testSetValueAction() {
	logger(LEVEL_INFO, "Start to test SetValueAction");
//	case_001();
//	case_002();
//	case_002();
//	case_003();
//	case_004();
//	case_005();
//	case_006();
//	case_007();
//	case_008();
//	case_009();

//	case_010();
//	case_011();
//	case_012();

//	case_013();
//	case_014();
//	case_015();
case_016();
}
