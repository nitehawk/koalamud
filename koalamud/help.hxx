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

#ifndef KOALA_HELP_HXX
#define KOALA_HELP_HXX "%A%"

#include "cmdtree.hxx"
#include "cmd.hxx"
#include "olc.hxx"

namespace koalamud {

/** Help OLC module
 * This Parser/OLC class handles editing help entries
 */
class HelpOLC : public olc
{
	public:
		HelpOLC(Char *ch, ParseDescriptor *pd, Parser *oldParser,
						long entry = 0);

	public:
		virtual void save(void);
		virtual bool load(void);
	
	protected:
		/** Help entry database ID */
		long int _entry;
		/** Help Entry Topic */
		QString _topic;
		/** Help Entry Keywords */
		QString _keywords;
		/** Help Entry Text */
		QString _body;
};
	
}; /* end koalamud namespace */

#endif //  KOALA_HELP_HXX
