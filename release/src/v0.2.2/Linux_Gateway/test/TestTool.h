/*
 * TestTool.h
 *
 *  Created on: Dec 25, 2016
 *      Author: clari
 */

#ifndef TESTTOOL_H_
#define TESTTOOL_H_

#include "mngModule/DeltaDoreServer.h"

#ifdef _WIN32
  #define TEST_ROOT_PATH "testinput/cmdExamples"
#else
  #define TEST_ROOT_PATH "/etc/luoping/save/cmdExamples"
#endif


extern void test_onMessage(string& key , string& message);

extern void test_onRequest(const string& message);

extern  const char* loadJson(const char* fileName);

extern void testSetValueAction();

extern TIME_TS TEST_DEVICE_TS;

#endif /* TESTTOOL_H_ */
