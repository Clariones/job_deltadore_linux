/*
 * TestTool.cpp
 *
 *  Created on: Dec 25, 2016
 *      Author: clari
 */


#include "TestTool.h"
TIME_TS TEST_DEVICE_TS;

void test_onMessage(string& key , string& message){
	pSubscribe->onSubscriptionMessage("tester", key, message);
}
void test_onRequest(const string& message){
	string response;
	pService->onRequest("tester",message,response);
	printf("[TEST_REQUEST]%s\n", response.c_str());
}

const char* loadJson(const char* fileName){
	char fileNameBuff[1024];
	sprintf(fileNameBuff,"%s/%s", TEST_ROOT_PATH, fileName);
	char* buff = readFileAsString(fileNameBuff);
	int pos = Helper::lastIndexOf(buff, "}");
	buff[pos+1] = 0;
	return buff;
}
