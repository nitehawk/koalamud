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

#define KOALA_ROOM_CXX "%A%"

#include <qregexp.h>

#include "room.hxx"
#include "logging.hxx"

namespace koalamud {

/** Load a room by its coordinate reference.
 * This is the standard constructor.  The default values will create a new
 * room for OLC, Any other coordinates load the room from the database or find
 * and setup the room from wilderness
 * @param lat Room latitude coordinate
 * @param longi Room longitude coordinate
 * @param ele Room elevation coordinate
 * @param zone Zone of the room, only used for new rooms
 */
Room::Room(int zone, int lat, int longi, int ele)
	: _virtual(false), _zone(zone), _lat(lat), _long(longi), _elev(ele)
{
	for (int d = DIR_NORTH; d <= DIR_DOWN; d++)
		exits[d] = NULL;

	load();

	RoomMap.insert(getRef(zone, lat, longi, ele), this);
}

/** Virtual Room (not saved) - used for generating mazes and other dynamic
 * rooms.
 * This constructor creates a room that will not be saved to the database for
 * various temporary usages.  These rooms might be used for generating dynamic
 * mazes and perhaps some forms of transportation.
 */
Room::Room(QString title, QString desc, QString smell, QString sound,
						QString soundfile,
						int zone, int lat, int longi, int elev,
						flags_t flags=FLAG_NOFLAGS, type_t rtype=TYPE_INDOORS,
						unsigned int plrlimit=0)
	: _title(title), _description(desc), _smell(smell), _sound(sound),
		_soundfile(soundfile), _virtual(true),
		_zone(zone), _lat(lat), _long(longi), _elev(elev),
		_plrlimit(plrlimit), _flags(flags), _rtype(rtype)
{
	for (int d = DIR_NORTH; d <= DIR_DOWN; d++)
		exits[d] = NULL;

	RoomMap.insert(getRef(zone, lat, longi, elev), this);
}

/** Destructor - Save room if needed and free memory */
Room::~Room()
{
	RoomMap.remove(getRef(_zone, _lat, _long, _elev));
}

/** Load a room from the database
 * @note This is only for non-virtual rooms
 */
bool Room::load()
{
	if (_virtual)
		return false;

	QString query;
	QTextOStream qos(&query);
	QSqlQuery q;

	qos << "select title, description, flags, type, plrlimit" << endl
			<< "from room where zone = " << _zone << endl
			<< "and latitude = " << _lat << " and longitude = " << _long << endl
			<< "and elevation = " << _elev << ";";
	if (q.exec(query) && q.numRowsAffected())
	{
		q.next();
		_title = q.value(0).toString();
		_description = q.value(1).toString();
		_plrlimit = q.value(4).toUInt();
		return true;
		/* Flags and type need to be interpreted */
	} else {
		QString log;
		QTextOStream out(&log);
		out << "Problem loading room " << getRef(_zone, _lat, _long, _elev);
		Logger::msg(log, Logger::LOG_ERROR);
		return false;
	}

}

/** Save room to the database */
bool Room::save()
{
	return false;
}

/** Display room to a character
 * This should check light levels, Character visibility and all the other
 * related things while it is building the display string */
QString Room::displayRoom(Char *ch, bool brief=false)
{
	QString disp;
	QTextOStream out(&disp);
	
	out << "|B-== |Y" << _title << " |B==-|x" << endl;
	
	if (!brief)
	{
		out << endl << _description << endl;
	}

	/* Display character short descriptions */
	QPtrListIterator<Char> curchar(charsinroom);
	for (; *curchar; ++curchar)
	{
		if (*curchar == ch)
			continue;

		out << (*curchar)->getShortDesc(ch) << endl;
	}

	/* List objects in room */
	
	return disp;
}

/** Send a message to everyone in the room */
void Room::sendToRoom(Char *from, Char *to, QString msg, QString fromtmpl,
				QString totmpl, QString roomtmpl)
{
	QPtrListIterator<Char> ch(charsinroom);
	QString out;

	for (; *ch; ++ch)
	{
		if (*ch == from)
		{
			out = fromtmpl.replace(QRegExp("%sender%"), "You");
			if (to)
				out = out.replace(QRegExp("%victim%"), to->getName(*ch));
			else
				out = out.replace(QRegExp("%victim%"), "");
		} else if (*ch == to)
		{
			out = totmpl.replace(QRegExp("%victim%"), "You");
			if (from)
				out = out.replace(QRegExp("%sender%"), from->getName(*ch));
			else
				out = out.replace(QRegExp("%sender%"), "");
		} else
		{
			if (from)
				out = roomtmpl.replace(QRegExp("%sender%"), from->getName(*ch));
			else
				out = roomtmpl.replace(QRegExp("%sender%"), "");
			if (to)
				out = out.replace(QRegExp("%victim%"), to->getName(*ch));
			else
				out = out.replace(QRegExp("%victim%"), "");
		}

		out = out.replace(QRegExp("%message%"), msg);

		(*ch)->sendtochar(out);

		if (*ch && *ch != from)
		{
			(*ch)->sendPrompt();
		}
	}
}

/** Change the coordinates of a room
 * This is used primarily in the Wilderness generation code
 */
void Room::moveRoom(int zone, int lat, int longi, int elev)
{
	RoomMap.remove(getRef(_zone, _lat, _long, _elev));
	_zone = zone;
	_lat = lat;
	_long = longi;
	_elev = elev;
	RoomMap.insert(getRef(zone, lat, longi, elev), this);
}

/** Find a room in the Room Map */
Room *Room::findRoom(int zone, int lat, int longi, int elev)
{
	return RoomMap[getRef(zone, lat, longi, elev)];
}

}; /* end koalamud namespace */
