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

#include "room.hxx"

/* {{{ K_Room Implementation */
/* {{{ Constructors */
/** Load a room by its coordinate reference.
 * This is the standard constructor.  The default values will create a new
 * room for OLC, Any other coordinates load the room from the database or find
 * and setup the room from wilderness
 * @param lat Room latitude coordinate
 * @param longi Room longitude coordinate
 * @param ele Room elevation coordinate
 * @param zone Zone of the room, only used for new rooms
 */
K_Room::K_Room(int lat=defnewlat, int longi=defnewlong, int ele=defnewele,
								unsigned int zone=1)
	: QObject(NULL, NULL), KRefObj(), _virtual(false),
		_lat(lat), _long(longi), _elev(ele),
		_zoneid(zone), _indatabase(false)
{
	if (lat == defnewlat && longi == defnewlong && ele == defnewele)
	{
		/* Construction is simple, just fill in default values for everything and
		 * return */
		_plrlimit = 0;
		_lvlmin = _lvlmax = 0;
		_title = "A new room";
		_description = "You are in a new room without a description.";
		_rtype = TYPE_CITY;
		_flags = FLAG_NOFLAGS;
	} else
	{
		/* We need to load the room from the database or construct the room from
		 * the wilderness areas */
	}
}

/** Virtual Room (not saved) - used for generating mazes and other dynamic
 * rooms.
 * This constructor creates a room that will not be saved to the database for
 * various temporary usages.  These rooms might be used for generating dynamic
 * mazes and perhaps some forms of transportation.
 */
K_Room::K_Room(QString title, QString desc, flags_t flags=FLAG_NOFLAGS, type_t rtype=TYPE_INDOORS)
	: QObject(NULL, NULL), KRefObj(), _title(title), _description(desc),
		_virtual(false), _flags(flags), _rtype(rtype), _zoneid(1),
		_indatabase(false)
{
}

/** Destructor - Save room if needed and free memory */
K_Room::~K_Room()
{
}
/* }}} Constructors */

/* {{{ Database Fuctions */
/** Actual database load, will also handle generating wilderness rooms */
bool K_Room::load()
{
	return false;
}

/** Save room to the database */
bool K_Room::save()
{
	return false;
}

/* }}} Database Fuctions */
/* }}} K_Room Implementation */
