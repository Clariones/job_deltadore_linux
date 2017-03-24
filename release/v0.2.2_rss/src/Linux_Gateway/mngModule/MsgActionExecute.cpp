/*
 * MsgActionExecutea.cpp
 *
 *  Created on: Dec 22, 2016
 *      Author: clari
 */

#include "MsgActionExecute.h"

MsgActionExecute::MsgActionExecute() {
	// TODO Auto-generated constructor stub

}

MsgActionExecute::~MsgActionExecute() {
	while (repeatActions.size() > 0){
		MsgActionExecute* pActionMsg = repeatActions.front();
		repeatActions.pop_front();
		if (pActionMsg != NULL){
			delete pActionMsg;
		}
	}
}

