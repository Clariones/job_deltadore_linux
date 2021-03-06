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

#ifndef REGISTERUNIT_H
#define REGISTERUNIT_H

#include "requirement.h"

namespace deltadoreX2d
{

class RegisterUnit
{
    public :
        RegisterUnit(int value);
        ~RegisterUnit();

        static const RegisterUnit KWH;
        static const RegisterUnit EURO;
		static const RegisterUnit WH;
        static RegisterUnit valueOf(int value);

        int toInt() const;
		const std::string& toString() const;

		bool operator==(const RegisterUnit &other) const;
		bool operator!=(const RegisterUnit &other) const;

    private :
		std::string m_name;
		int m_value;
};
}

#endif /* REGISTERUNIT_H */
