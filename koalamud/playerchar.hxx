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

#ifndef KOALA_PLAYERCHAR_HXX
#define KOALA_PLAYERCHAR_HXX "%A%"

#include <qobject.h>
#include <qlistview.h>
#include <qptrlist.h>
#include <qptrqueue.h>

#include <zthread/FastRecursiveMutex.h>
#include <zthread/Guard.h>
#include <zthread/Thread.h>

#include "main.hxx"
#include "char.hxx"
#include "network.hxx"
#include "koalastatus.h"
#include "cmd.hxx"
#include "comm.hxx"

namespace koalamud {

/** Player Character
 * This class is used for all normal players connected to the system.
 */
class PlayerChar : public Char
{
	Q_OBJECT

	public:
		PlayerChar(QString name, ParseDescriptor *desc=NULL);
		virtual ~PlayerChar(void);
		virtual bool load(void);
		virtual bool save(void);

	public:
		virtual void setDesc(ParseDescriptor *desc);
	
	public slots:
		virtual void descriptorClosed(void);

	protected:
		/** Database ID - 0 means we aren't in the database */
		int dbid;
};
	
}; /* end koalamud namespace */

/* Two lists of players used, one linked list and one hashmap */
/** Type of connected player list */
typedef QPtrList<koalamud::PlayerChar> playerlist_t;
/** Type of connected player list iterator */
typedef QPtrListIterator<koalamud::PlayerChar> playerlistiterator_t;

#ifdef KOALA_PLAYERCHAR_CXX
/** This lists all connected players */
playerlist_t connectedplayerlist;
/** This lists all logged in players */
QDict<koalamud::PlayerChar> connectedplayermap(101, false);
#else
extern playerlist_t connectedplayerlist;
extern QDict<koalamud::PlayerChar> connectedplayermap;
#endif

#endif  //  KOALA_PLAYERCHAR_HXX
