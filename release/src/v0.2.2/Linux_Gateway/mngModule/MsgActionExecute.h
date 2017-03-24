/*
 * MsgActionExecutea.h
 *
 *  Created on: Dec 22, 2016
 *      Author: clari
 */

#ifndef MSGACTIONEXECUTEA_H_
#define MSGACTIONEXECUTEA_H_

#include <string>
#include <list>

using namespace std;

class MsgActionExecute {
public:
	MsgActionExecute();
	virtual ~MsgActionExecute();

public:
	// original input fields from subscribe message
	string id;
	string type;
	string value;
	string count;
	string delay_on;
	string delay_off;
	string period;
	string delaytime;
	string clusterid;
	// parsed C style value from message
	int action_type;
	bool value_onOff;
	int value_level;
	int value_count;
	int value_delay_on;
	int value_delay_off;
	int value_period;
	int value_delaytime;
	bool value_enable;
	list<MsgActionExecute*> repeatActions;
};

#endif /* MSGACTIONEXECUTEA_H_ */
