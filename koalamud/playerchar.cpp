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
	: Char(name, NULL), dbid(0)
{

	if (desc)
		setDesc(desc);
	
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

	save();

	/* cleanup gui */
	if (srv->usegui())
	{
	}
}

/** Handle closing descriptor
 * This tags the player as linkless and starts a counter.  When the counter
 * reaches a specific threhold, the player is logged off completely.
 */
void PlayerChar::descriptorClosed(void)
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
	QSqlQuery q;
	int inzone, inlat, inlong, inelev;

	{
		QString query;
		QTextOStream qos(&query);

		qos << "select playerid, name, lastname, inroomzone, inroomlat, "
				<< "inroomlong, inroomelev from players" << endl
				<< "where name = '" << _name << "';";
		if (q.exec(query) && q.next())
		{
			dbid = q.value(0).toInt();
			_name = q.value(1).toString();
			_lname = q.value(2).toString();
			inzone = q.value(3).toInt();
			inlat = q.value(4).toInt();
			inlong = q.value(5).toInt();
			inelev = q.value(6).toInt();
		} else {
			cerr << "Failed query: " << query << endl;
			inzone = inlat = inlong = inelev = 0;
		}
	}

	/* Load skills */
	if (dbid != 0)
	{
		QString query;
		QTextOStream qos(&query);

		qos << "select skid, learned" << endl
				<< "from skilllevels" << endl
				<< "where pid = " << dbid << ";";
		if (q.exec(query))
		{
			while(q.next())
			{
				setSkillLevel(q.value(0).toString(), q.value(1).toInt());
			}
		} else {
			cerr << "Failed query: " << query << endl;
		}
	}

	/* turn on color */
	_desc->setColor(true);
	_inroom = Room::findRoom(inzone, inlat, inlong, inelev);
	return true;
}

/** Save player to database */
bool PlayerChar::save(void)
{
	if (dbid == 0)
		return false;

	QSqlQuery q;
	{
		/* Save main player record */
		QString query;
		QTextOStream qos(&query);

		qos << "update players" << endl
				<< "set lastlogin = NOW()," << endl
				<< "inroomzone = " << _inroom->getZone() << "," << endl
				<< "inroomlat = " << _inroom->getLat() << "," << endl
				<< "inroomlong = " << _inroom->getLong() << "," << endl
				<< "inroomelev = " << _inroom->getElev() << endl
				<< "where playerid = " << dbid << ";";

		if (!q.exec(query))
			cerr << "Failed query: " << query << endl;
	}

	{
		/* Save skill levels */
		QString query;
		QTextOStream qos(&query);

		qos << "replace into skilllevels (pid, skid, learned) values";
		QDictIterator<SkillRecord> skrec(skills);
		bool first=true;
		for (; *skrec; ++skrec)
		{
			if (!first)
			{
				qos << ",";
			} else {
				first = false;
			}
			qos << endl << "(" << dbid << ", '" << (*skrec)->getId() << "',"
					<< (*skrec)->getKnow() << ")";
		}
		qos << ";";

		if (!q.exec(query))
			cerr << "Failed query: " << query << endl;
	}
	return true;
}

}; /* end koalamud namespace */
