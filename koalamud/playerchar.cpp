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

}; /* end koalamud namespace */
