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
** ylebret             | 280212           | refactoring
*/

#ifndef COMPOSITEREGISTERSELECTION_H
#define COMPOSITEREGISTERSELECTION_H

#include "requirement.h"

namespace deltadoreX2d
{

class CompositeRegisterSelection
{
    public :
        CompositeRegisterSelection(int value) ;
        ~CompositeRegisterSelection();

        static const CompositeRegisterSelection HEATING__HOT_WATER__TOTAL;
        static const CompositeRegisterSelection CURRENT_TRANSFORMERS_1_2_3;
        static CompositeRegisterSelection valueOf(int value);

        int toInt() const;
		const std::string& toString() const;

		bool operator==(const CompositeRegisterSelection &other) const;
		bool operator!=(const CompositeRegisterSelection &other) const;

   private :
		std::string m_name;
		int m_value;
};
}

#endif /* COMPOSITEREGISTERSELECTION_H */
