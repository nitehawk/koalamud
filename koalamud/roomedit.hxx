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

#ifndef KOALA_ROOMEDIT_HXX
#define KOALA_ROOMEDIT_HXX "%A%"

#include "olc.hxx"

namespace koalamud
{

/** Room OLC Module
 * This OLC handles room editing
 * @todo Add code to OLC to handle editing exits
 */
class RoomOLC : public olc
{
	public:
		RoomOLC(Char *ch, ParseDescriptor *pd, Parser *oldParser,
						int zone, int lat, int longi, int elev);

	public:
		virtual void save(void);
		virtual bool load(void);
	
	protected:
		/** Room zone */
		long ezone;
		/** Room lat */
		long elat;
		/** Room long */
		long elong;
		/** Room elev */
		long eelev;
		/** QString Room Title */
		QString title;
		/** Room flags */
		long flags;
		/** Room Type */
		long type;
		/** Player limit */
		long plrlimit;
		/** light level */
		long lightlev;
		/** Room Description */
		QString description;
};

}; /* end koalamud namespace */

#endif //  KOALA_ROOMEDIT_HXX
