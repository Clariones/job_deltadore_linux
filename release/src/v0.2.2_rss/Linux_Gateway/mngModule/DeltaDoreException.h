/*
 * DeltaDoreException.h
 *
 *  Created on: Dec 17, 2016
 *      Author: clari
 */

#ifndef DELTADOREEXCEPTION_H_
#define DELTADOREEXCEPTION_H_

#define ERR_CODE_LOADCONFIG -1
#define ERR_CODE_PARSE_JSON -2
#define ERR_CODE_MSG_MEMBER_NULL -3
#define ERR_CODE_MSG_ACTIONS_NULL -4
#define ERR_CODE_MSG_INVALID_ACTIONS -5
#define ERR_CODE_MSG_UNSUPPORT_TYPE -6
#define ERR_CODE_MSG_NESTED_REPEAT -7

#define ERR_CODE_DRIVER_INIT_FAIL -101

#define ERR_CODE_CMD_INVALID_ID -201


class DeltaDoreException {
public:
	DeltaDoreException():mCode(-1) {}
	virtual ~DeltaDoreException() {}
	DeltaDoreException(int code) : mCode(code){}

public:
	int getCode() { return mCode;}

protected:
	int mCode;
};

#endif /* DELTADOREEXCEPTION_H_ */
