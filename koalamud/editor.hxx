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

#ifndef KOALA_EDITOR_HXX
#define KOALA_EDITOR_HXX "%A%"

#include "olc.hxx"
#include "cmd.hxx"

namespace koalamud
{

/** Line editor class
 * This class provides a basic line editor class.  It also serves as a base
 * class for other editor types to implement new key bindings or behaviour
 * @note Eventually we need to setup a factory type environment for
 * instantiating editor objects.  Until that is done, there won't be any way
 * to tag different editors as a preference item.
 */
class Editor : public Parser
{
	public:
		/** Editor State */
		typedef enum {
			STATE_MENU, /**< We are in the menu */
			STATE_APPEND, /**< We are appending lines */
			STATE_INSERT, /**< We are inserting lines */
		} state_t;
	public:
		Editor(Char *ch, ParseDescriptor *pd, Parser *oldParser,
						Command *postcmd, QString initial = "", bool sendinitial = true);
		Editor(Char *ch, ParseDescriptor *pd, olc *activeolc,
						QString initial="", bool sendinitial = true);

	public:
		virtual void parseLine(QString line);
		virtual void returnControl(bool aborting=false);
		virtual void sendHelp(void);

	protected:
		/** True if we were spawned from an OLC environment */
		bool isOLC;
		/** Pointer to parser that spawned us */
		Parser *_old;
		/** Pointer to command to run when we are done */
		Command *_postcmd;
		/** Lines of input in the editor */
		QStringList lines;
		/** Endline string */
		QString el;
		/** Current editor state */
		state_t curstate;
		/** Insert position - used in STATE_INSERT */
		int insertpos;
};
	
}; /* end koalamud namespace */

#endif //  KOALA_EDITOR_HXX
