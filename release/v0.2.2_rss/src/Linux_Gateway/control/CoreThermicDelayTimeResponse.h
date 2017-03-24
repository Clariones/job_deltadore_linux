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
** mleforestier        | 041811           | First version
** ylebret             | 280212           | refactoring
*/

#ifndef CORETHERMICDELAYTIMERESPONSE_H
#define CORETHERMICDELAYTIMERESPONSE_H

#include "control/CoreResponse.h"
#include "hvac/ThermicDelayTimeResponse.h"

namespace deltadoreX2d
{

class CoreThermicDelayTimeResponse : public CoreResponse, public virtual ThermicDelayTimeResponse
{
    public :
        CoreThermicDelayTimeResponse(const ThermicDelayTime& delayTime);
        ~CoreThermicDelayTimeResponse();

		const ThermicDelayTime& getDelayTime() const;
		bool instanceOf(ResponseClass type) const;
		Response* clone() const;
		void* derived();

	private :
		ThermicDelayTime m_delayTime;
};
}

#endif // CORETHERMICDELAYTIMERESPONSE
