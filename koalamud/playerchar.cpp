/***************************************************************\
*                        KoalaMud Gen 2                         *
*    Copyright (c) 2002 First Step Internet Services, Inc.      *
*                     All Rights Reserved                       *
*        Distributed under the terms of the FSI License         *
* See file LICENSE in the root of this package for information  *
\***************************************************************/
/***************************************************************\
* Module:
* Author:
* Description:
* Classes:
\***************************************************************/

#define KOALA_PLAYERCHAR_CXX "%A%"

#include <qregexp.h>
#include <qsqldatabase.h>

#include <iostream>

#include <unistd.h>

#include "main.hxx"
#include "event.hxx"
#include "playerchar.hxx"
#include "cmdtree.hxx"
#include "logging.hxx"
#include "room.hxx"

namespace koalamud {

/** Build Player character object, including loading from the database */
PlayerChar::PlayerChar(QString name, ParseDescriptor *desc = NULL)
	: Char(name, desc)
{
	if (srv->usegui())
	{
	}

	/* Load from the database */
	load();

	/* Add ourself to the player lists */
	connectedplayerlist.append(this);
	connectedplayermap.insert(name, this);
	/* Join gossip channel if it exists */
	if (channelmap["gossip"] != NULL)
	{
		channelmap["gossip"]->joinchannel(this);
	}

	/* Log a message */
	QString str;
	QTextOStream os(&str);
	os << name << " is now connected.";
	Logger::msg(str);

	if (_inroom)
	{
		_inroom->enterRoom(this);
		sendtochar(_inroom->displayRoom(this, false));
	}
}

/** Destroy a player character object */
PlayerChar::~PlayerChar(void)
{
	/* Remove ourself from the player lists */
	connectedplayerlist.removeRef(this);
	connectedplayermap.remove(_name);

	/* cleanup gui */
	if (srv->usegui())
	{
	}
}

/** Handle closing descriptor
 * This tags the player as linkless and starts a counter.  When the counter
 * reaches a specific threhold, the player is logged off completely.
 */
void PlayerChar::descriptorClosing(void)
{
	/* Until we actually get the event/tick thread setup, we just log off the
	 * player (delete player object).
	 * FIXME
	 */
	_desc = NULL;
	delete this;
}

/** Change the descriptor attached to this PC
 * This will disconnect from the old descriptor and delete it and link to the
 * new descriptor
 */
void PlayerChar::setDesc(ParseDescriptor *desc)
{
	disconnect(_desc);
	delete _desc;
	_desc = desc;
	connect(desc, SIGNAL(destroyed()), this, SLOT(descriptorClosed()));
}

/** Load player from database */
bool PlayerChar::load(void)
{
	/* turn on color */
	_desc->setColor(true);
	_inroom = Room::findRoom(0,0,0,0);
	return false;
}

/** Save player to database */
bool PlayerChar::save(void)
{
	return false;
}

}; /* end koalamud namespace */
