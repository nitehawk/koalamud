/***************************************************************\
*                        KoalaMud Gen 2                         *
*    Copyright (c) 2002 First Step Internet Services, Inc.      *
*                     All Rights Reserved                       *
*        Distributed under the terms of the FSI License         *
* See file LICENSE in the root of this package for information  *
\***************************************************************/
/***************************************************************\
*	Module:
*	Author:
* Description:
* Classes:
\***************************************************************/

#ifndef KOALA_EVENT_HXX
#define KOALA_EVENT_HXX "%A%"

#include <qevent.h>
#include <qthread.h>

#include "char.hxx"

#define EVENT_CHAR_OUTPUT		QEvent::User + 1

namespace koalamud {
	/** Char output event */
	class CharOutputEvent : public QCustomEvent
	{
		public:
			/** Build Character Output event */
			CharOutputEvent(Char *ch) : QCustomEvent(EVENT_CHAR_OUTPUT), _ch(ch)
					{ }
			/** Pointer to character object */
			Char *_ch;
	};

}; /* end koalamud namespace */

#endif //  KOALA_EVENT_HXX
