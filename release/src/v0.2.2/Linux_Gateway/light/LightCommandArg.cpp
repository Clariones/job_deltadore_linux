/*
** Copyright (C) 2010 Eff'Innov Technologies.  All rights reserved.
** May not be redistributed without prior written permission.
**
** Based on Java version by DeltaDore, subject to DeltaDore copyrights
**
** Note: Eff'Innov Technologies disclaims responsibility for any malfunction
** or error that may arise from any change to the current file, provided that such
** change have not been submitted to and formerly approved by Eff'Innov Technologies,
** prior to the occurence of such malfunction or error
**
** Author: Mickael Leforestier (mickael.leforestier@effinnov.com)
**
** History log:
** ------------------------------------------------------------------------------
** Author              | Date             | Changes
** ------------------------------------------------------------------------------
** mleforestier        | 092410           | First version
** ylebret             | 280212           | refactoring
*/

#include "light/LightCommandArg.h"

namespace deltadoreX2d
{

LightCommandArg::LightCommandArg(int value)
{
	m_value = value;
	if (value == 0) { m_name = "DOWN"; }
	else if (value == 100) { m_name = "UP"; }
	else if (value == 101) { m_name = "STOP"; }
	else if (value == 102) { m_name = "UP_SLOW"; }
	else if (value == 103) { m_name = "DOWN_SLOW"; }
	else if (value == 104) { m_name = "GO_FAVORITE_1"; }
	else if (value == 105) { m_name = "ALARM_PAIRING"; }
	else if (value == 106) { m_name = "STAND_OUT"; }
	else if (value == 107) { m_name = "GO_FAVORITE_2"; }
	else if (value == 108) { m_name = "TOGGLE"; }
	else if (value == 109) { m_name = "NA"; }
	else if (value > 0 && value < 100) { m_name = value + "%"; }
	else { throw X2dException("Invalid argument => LightCommandArg::LightCommandArg(int value)"); }
}

LightCommandArg::~LightCommandArg()
{
    //
}

const LightCommandArg LightCommandArg::DOWN = LightCommandArg(0);
const LightCommandArg LightCommandArg::UP = LightCommandArg(100);
const LightCommandArg LightCommandArg::STOP = LightCommandArg(101);
const LightCommandArg LightCommandArg::UP_SLOW = LightCommandArg(102);
const LightCommandArg LightCommandArg::DOWN_SLOW = LightCommandArg(103);
const LightCommandArg LightCommandArg::GO_FAVORITE_1 = LightCommandArg(104);
const LightCommandArg LightCommandArg::ALARM_PAIRING = LightCommandArg(105);
const LightCommandArg LightCommandArg::STAND_OUT = LightCommandArg(106);
const LightCommandArg LightCommandArg::GO_FAVORITE_2 = LightCommandArg(107);
const LightCommandArg LightCommandArg::TOGGLE = LightCommandArg(108);
const LightCommandArg LightCommandArg::NA = LightCommandArg(109);

LightCommandArg LightCommandArg::valueOf(int value)
{
    return LightCommandArg(value);
}

LightCommandArg LightCommandArg::percent(int value)
{
    if (value >= 0 && value <= 100)
    {
        return LightCommandArg(value);
    }
	throw X2dException("Invalid argument => LightCommandArg::percent(int value)");
}

int LightCommandArg::toInt() const
{
    return m_value;
}

const std::string& LightCommandArg::toString() const
{
	return m_name;
}

bool LightCommandArg::instanceOf(ArgClass type) const
{
	if (type == LightCommandArg_t) { return true; }
	return false;
}

NodeArg* LightCommandArg::clone() const
{
	return new LightCommandArg(*this);
}

void* LightCommandArg::derived()
{
	return this;
}

bool LightCommandArg::operator==(const LightCommandArg &other) const {
	return (m_value == other.toInt());
}

bool LightCommandArg::operator!=(const LightCommandArg &other) const {
	return !(*this == other);
}
}
