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
#include "roomedit.hxx"
#include "logging.hxx"
#include "cmdtree.hxx"

namespace koalamud {
/** Map roomexit strings to their flag number */
QMap<QString, RoomExit::flag_t> RoomExitstringtoflagmap;
/** Map roomexit flag numbers to their string */
static QMap<RoomExit::flag_t, QString> RoomExitflagtostringmap;

/** Map Room strings to their flag number */
static QMap<QString, Room::flags_t> Roomstringtoflagmap;
/** Map Room flag numbers to their string */
static QMap<Room::flags_t, QString> Roomflagtostringmap;
/** Map Room strings to their direction number */
static QMap<QString, Room::directions> Roomstringtodirmap;
/** Map direction numbers to their string */
static QMap<Room::directions, QString> Roomdirtostringmap;
/** Map direction to its opposite */
static QMap<Room::directions, Room::directions> Roomdiroppositemap;


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
						unsigned int flags=0, type_t rtype=TYPE_INDOORS,
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

	for (int d = DIR_NORTH; d <= DIR_DOWN; d++)
		delete exits[d];
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

	qos << "select title, description, flags+0, type+0, plrlimit" << endl
			<< "from room where zone = " << _zone << endl
			<< "and latitude = " << _lat << " and longitude = " << _long << endl
			<< "and elevation = " << _elev << ";";
	if (q.exec(query) && q.numRowsAffected())
	{
		q.next();
		_title = q.value(0).toString();
		_description = q.value(1).toString();
		_plrlimit = q.value(4).toUInt();
		_flags = q.value(2).toUInt();
		_rtype = (type_t)q.value(3).toInt();
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
	
	/* Note, the compass code is going to need a lot of changes later on to
	 * account for light, closed doors, hidden exists, and a variety of other
	 * stuff. */
	bool compass = ch->showCompass();

	if (compass)
	{
		out << " ";
		if (exits[DIR_NORTHWEST] && exits[DIR_NORTHWEST]->isVisible())
		{
			if (exits[DIR_NORTHWEST]->isSet(RoomExit::FLAG_CLOSED))
			{
				out << "|M# ";
			} else {
				/* Still need to account for light level */
				out << "|YNW";
			}
		} else {
			out << "|B- ";
		}
		out << "     ";
		if (exits[DIR_NORTH] && exits[DIR_NORTH]->isVisible())
		{
			if (exits[DIR_NORTH]->isSet(RoomExit::FLAG_CLOSED))
			{
				out << "|M#";
			} else {
				/* Still need to account for light level */
				out << "|YN";
			}
		} else {
			out << "|B-";
		}
		out << "     ";
		if (exits[DIR_NORTHEAST] && exits[DIR_NORTHEAST]->isVisible())
		{
			if (exits[DIR_NORTHEAST]->isSet(RoomExit::FLAG_CLOSED))
			{
				out << " |M#";
			} else {
				/* Still need to account for light level */
				out << "|YNE";
			}
		} else {
			out << " |B-";
		}
		out << "  ";
	}
	out << "|Y" << _title << "|x" << endl;

	if (compass)
	{
		out << " ";
		if (exits[DIR_WEST] && exits[DIR_WEST]->isVisible())
		{
			if (exits[DIR_WEST]->isSet(RoomExit::FLAG_CLOSED))
			{
				out << "|M#";
			} else {
				/* Still need to account for light level */
				out << "|YW";
			}
		} else {
			out << "|B-";
		}
		out << " |B<-";
		if (exits[DIR_UP] && exits[DIR_UP]->isVisible())
		{
			if (exits[DIR_UP]->isSet(RoomExit::FLAG_CLOSED))
			{
				out << "|M#";
			} else {
				/* Still need to account for light level */
				out << "|YU";
			}
		} else {
			out << "-";
		}
		out << "|B-[|MM|B]-";
		if (exits[DIR_DOWN] && exits[DIR_DOWN]->isVisible())
		{
			if (exits[DIR_DOWN]->isSet(RoomExit::FLAG_CLOSED))
			{
				out << "|M#";
			} else {
				/* Still need to account for light level */
				out << "|YD";
			}
		} else {
			out << "-";
		}
		out << "|B-> ";
		if (exits[DIR_EAST] && exits[DIR_EAST]->isVisible())
		{
			if (exits[DIR_EAST]->isSet(RoomExit::FLAG_CLOSED))
			{
				out << "|M#";
			} else {
				/* Still need to account for light level */
				out << "|YE";
			}
		} else {
			out << "-";
		}
		out << " ";
	}
	
	out << "|B-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|x" << endl;

	if (compass)
	{
		out << " ";
		if (exits[DIR_SOUTHWEST] && exits[DIR_SOUTHWEST]->isVisible())
		{
			if (exits[DIR_SOUTHWEST]->isSet(RoomExit::FLAG_CLOSED))
			{
				out << "|M# ";
			} else {
				/* Still need to account for light level */
				out << "|YSW";
			}
		} else {
			out << "|B- ";
		}
		out << "     ";
		if (exits[DIR_SOUTH] && exits[DIR_SOUTH]->isVisible())
		{
			if (exits[DIR_SOUTH]->isSet(RoomExit::FLAG_CLOSED))
			{
				out << "|M#";
			} else {
				/* Still need to account for light level */
				out << "|YS";
			}
		} else {
			out << "|B-";
		}
		out << "     ";
		if (exits[DIR_SOUTHEAST] && exits[DIR_SOUTHEAST]->isVisible())
		{
			if (exits[DIR_SOUTHEAST]->isSet(RoomExit::FLAG_CLOSED))
			{
				out << " |M#";
			} else {
				/* Still need to account for light level */
				out << "|YSE";
			}
		} else {
			out << " |B-|x";
		}
		out << " ";
	}

	/* If the character is an immortal, we'll show room flags as well */
	if (ch->isImmortal())
	{
		int count = 0;
		for (unsigned int i = FLAG_LEVELRESTRICT; i < FLAG_ENDLIST; ++i)
		{
			if (_flags[i])
			{
				if (count)
				{
					out << ", ";
				}
				out << Roomflagtostringmap[(flags_t) i];
				++count;
			}
		}
	}
	out << endl;
	
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
				QString totmpl, QString roomtmpl, bool sendPrompt=true)
{
	QPtrListIterator<Char> ch(charsinroom);
	QString out;

	for (; *ch; ++ch)
	{
		if (*ch == from)
		{
			if (fromtmpl.length() < 2)
				continue;
			out = fromtmpl.replace(QRegExp("%sender%"), "You");
			if (to)
				out = out.replace(QRegExp("%victim%"), to->getName(*ch));
			else
				out = out.replace(QRegExp("%victim%"), "");
		} else if (*ch == to)
		{
			if (totmpl.length() < 2)
				continue;
			out = totmpl.replace(QRegExp("%victim%"), "You");
			if (from)
				out = out.replace(QRegExp("%sender%"), from->getName(*ch));
			else
				out = out.replace(QRegExp("%sender%"), "");
		} else
		{
			if (roomtmpl.length() < 2)
				continue;
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

		if (sendPrompt && *ch != from)
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

/** Initialize Mappings for Room enum types */
/* {{{ */
void Room::initializeMaps(void)
{
	static bool done = false;
	if (done)
		return;

	/* Flags */
	Roomstringtoflagmap["levelrest"] = FLAG_LEVELRESTRICT;
	Roomflagtostringmap[FLAG_LEVELRESTRICT] = "levelrest";
	Roomstringtoflagmap["regen"] = FLAG_REGEN;
	Roomflagtostringmap[FLAG_REGEN] = "regen";
	Roomstringtoflagmap["deathtrap"] = FLAG_DEATHTRAP;
	Roomflagtostringmap[FLAG_DEATHTRAP] = "deathtrap";
	Roomstringtoflagmap["nonpc"] = FLAG_NONPC;
	Roomflagtostringmap[FLAG_NONPC] = "nonpc";
	Roomstringtoflagmap["safe"] = FLAG_SAFE;
	Roomflagtostringmap[FLAG_SAFE] = "safe";
	Roomstringtoflagmap["savespot"] = FLAG_SAVESPOT;
	Roomflagtostringmap[FLAG_SAVESPOT] = "savespot";
	Roomstringtoflagmap["recallspot"] = FLAG_RECALLSPOT;
	Roomflagtostringmap[FLAG_RECALLSPOT] = "recallspot";
	Roomstringtoflagmap["notrack"] = FLAG_NOTRACK;
	Roomflagtostringmap[FLAG_NOTRACK] = "notrack";
	Roomstringtoflagmap["nomagic"] = FLAG_NOMAGIC;
	Roomflagtostringmap[FLAG_NOMAGIC] = "nomagic";
	Roomstringtoflagmap["noteleport"] = FLAG_NOTELEPORT;
	Roomflagtostringmap[FLAG_NOTELEPORT] = "noteleport";
	Roomstringtoflagmap["dizzy"] = FLAG_DIZZY;
	Roomflagtostringmap[FLAG_DIZZY] = "dizzy";

	/* Directions */
	Roomstringtodirmap["NORTH"] = DIR_NORTH;
	Roomdirtostringmap[DIR_NORTH] = "NORTH";
	Roomstringtodirmap["NORTHWEST"] = DIR_NORTHWEST;
	Roomdirtostringmap[DIR_NORTHWEST] = "NORTHWEST";
	Roomstringtodirmap["WEST"] = DIR_WEST;
	Roomdirtostringmap[DIR_WEST] = "WEST";
	Roomstringtodirmap["SOUTHWEST"] = DIR_SOUTHWEST;
	Roomdirtostringmap[DIR_SOUTHWEST] = "SOUTHWEST";
	Roomstringtodirmap["SOUTH"] = DIR_SOUTH;
	Roomdirtostringmap[DIR_SOUTH] = "SOUTH";
	Roomstringtodirmap["SOUTHEAST"] = DIR_SOUTHEAST;
	Roomdirtostringmap[DIR_SOUTHEAST] = "SOUTHEAST";
	Roomstringtodirmap["EAST"] = DIR_EAST;
	Roomdirtostringmap[DIR_EAST] = "EAST";
	Roomstringtodirmap["NORTHEAST"] = DIR_NORTHEAST;
	Roomdirtostringmap[DIR_NORTHEAST] = "NORTHEAST";
	Roomstringtodirmap["UP"] = DIR_UP;
	Roomdirtostringmap[DIR_UP] = "UP";
	Roomstringtodirmap["DOWN"] = DIR_DOWN;
	Roomdirtostringmap[DIR_DOWN] = "DOWN";

	Roomdiroppositemap[DIR_NORTH] = DIR_SOUTH;
	Roomdiroppositemap[DIR_NORTHEAST] = DIR_SOUTHWEST;
	Roomdiroppositemap[DIR_EAST] = DIR_WEST;
	Roomdiroppositemap[DIR_SOUTHEAST] = DIR_NORTHWEST;
	Roomdiroppositemap[DIR_SOUTH] = DIR_NORTH;
	Roomdiroppositemap[DIR_SOUTHWEST] = DIR_NORTHEAST;
	Roomdiroppositemap[DIR_WEST] = DIR_EAST;
	Roomdiroppositemap[DIR_NORTHWEST] = DIR_SOUTHEAST;
	Roomdiroppositemap[DIR_UP] = DIR_DOWN;
	Roomdiroppositemap[DIR_DOWN] = DIR_UP;
}
/* }}} */

/** Load all the rooms in the world and generate wilderness and other
 * generated zones.
 */
void Room::loadWorldRooms(void)
{
	Room::initializeMaps();
	RoomExit::initializeMaps();

	QString query("select zone, latitude, longitude, elevation from room");
	QSqlQuery q;

	/* Load rooms */
	q.exec(query);
	while (q.next())
	{
		new Room(q.value(0).toInt(), q.value(1).toInt(), q.value(2).toInt(),
							q.value(3).toInt());
	}

	/* Load exits */
	QTextOStream qos(&query);
	qos << "select r1zone, r1lat, r1long, r1elev," << endl
			<< "r2zone, r2lat, r2long, r2elev, name, keyobj, flags+0," << endl
			<< "direction from roomexits;";
	q.exec(query);
	while (q.next())
	{
		RoomExit::makeExits(q.value(0).toInt(), q.value(1).toInt(),
						q.value(2).toInt(), q.value(3).toInt(),
						q.value(4).toInt(), q.value(5).toInt(),
						q.value(6).toInt(), q.value(7).toInt(),
						q.value(8).toString(), q.value(9).toInt(),
						q.value(10).toUInt(), q.value(11).toString());
	}
}

/** Create exits and attach them to the appropriate rooms */
void RoomExit::makeExits(int r1zone, int r1lat, int r1long, int r1elev,
												 int r2zone, int r2lat, int r2long, int r2elev,
												 QString name, int keyobj, unsigned int flagval,
												 QString dir)
{
	Room *r1 = Room::findRoom(r1zone, r1lat, r1long, r1elev);
	Room *r2 = Room::findRoom(r2zone, r2lat, r2long, r2elev);

	/* Make sure we have both room pointers */
	if (!r1 || !r2)
	{
		return;
	}
	
	bool twoway = false;
	if (flagval & (1 << FLAG_TWOWAY))
	{
		twoway = true;
	}

	Room::directions direction = Roomstringtodirmap[dir.upper()];
	Room::directions odir = Roomdiroppositemap[direction];

	r1->attachExit(new RoomExit(r2, flagval, name, keyobj), direction);
	if (twoway)
		r2->attachExit(new RoomExit(r1, flagval, name, keyobj), odir);
}

/** Initialize flag to string maps */
/* {{{ */
void RoomExit::initializeMaps(void)
{
	static bool done = false;
	if (done)
		return;
	RoomExitstringtoflagmap["DOOR"] = FLAG_DOOR;
	RoomExitflagtostringmap[FLAG_DOOR] = "DOOR";
	RoomExitstringtoflagmap["LOCKABLE"] = FLAG_LOCKABLE;
	RoomExitflagtostringmap[FLAG_LOCKABLE] = "LOCKABLE";
	RoomExitstringtoflagmap["LOCKED"] = FLAG_LOCKED;
	RoomExitflagtostringmap[FLAG_LOCKED] = "LOCKED";
	RoomExitstringtoflagmap["PICKPROOF"] = FLAG_PICKPROOF;
	RoomExitflagtostringmap[FLAG_PICKPROOF] = "PICKPROOF";
	RoomExitstringtoflagmap["CLOSED"] = FLAG_CLOSED;
	RoomExitflagtostringmap[FLAG_CLOSED] = "CLOSED";
	RoomExitstringtoflagmap["MAGICLOCK"] = FLAG_MAGICLOCK;
	RoomExitflagtostringmap[FLAG_MAGICLOCK] = "MAGICLOCK";
	RoomExitstringtoflagmap["HIDDEN"] = FLAG_HIDDEN;
	RoomExitflagtostringmap[FLAG_HIDDEN] = "HIDDEN";
	RoomExitstringtoflagmap["TRAPPED"] = FLAG_TRAPPED;
	RoomExitflagtostringmap[FLAG_TRAPPED] = "TRAPPED";
	RoomExitstringtoflagmap["MAGICPROOF"] = FLAG_MAGICPROOF;
	RoomExitflagtostringmap[FLAG_MAGICPROOF] = "MAGICPROOF";
	RoomExitstringtoflagmap["BASHPROOF"] = FLAG_BASHPROOF;
	RoomExitflagtostringmap[FLAG_BASHPROOF] = "BASHPROOF";
	RoomExitstringtoflagmap["TWOWAY"] = FLAG_TWOWAY;
	RoomExitflagtostringmap[FLAG_TWOWAY] = "TWOWAY";
} /* }}} */

/** Setup a room OLC */
RoomOLC::RoomOLC(Char *ch, ParseDescriptor *pd, Parser *oldParser,
								int zone, int lat, int longi, int elev)
	: olc(ch, pd, oldParser), ezone(zone), elat(lat), elong(longi), eelev(elev)
{
	/* Add fields */
	addField(QString("Zone Number"), FIELD_INTEGER, &ezone, false);
	addField(QString("Latitude"), FIELD_INTEGER, &elat, false);
	addField(QString("Longitude"), FIELD_INTEGER, &elong, false);
	addField(QString("Elevation"), FIELD_INTEGER, &eelev, false);
	addField(QString("Room Title"), FIELD_STRING, &title, true, 50);
	addField(QString("Room Description"), FIELD_MULTILINE, &description, true);
	addField(QString("Player Limit"), FIELD_INTEGER, &plrlimit, true, 1000, 0);
	addField(QString("Light Level"), FIELD_INTEGER, &lightlev, true, 100, 0);

	/* Room Flags */
	field_t *field = addField(QString("Flags"), FIELD_SET, &flags, true);
	addFlag(field, Room::FLAG_LEVELRESTRICT, QString("Level Restrict"));
	addFlag(field, Room::FLAG_REGEN, QString("Regen"));
	addFlag(field, Room::FLAG_DEATHTRAP, QString("Deathtrap"));
	addFlag(field, Room::FLAG_NONPC, QString("No Mob"));
	addFlag(field, Room::FLAG_SAFE, QString("Safe"));
	addFlag(field, Room::FLAG_SAVESPOT, QString("Save Spot"));
	addFlag(field, Room::FLAG_RECALLSPOT, QString("Recall Spot"));
	addFlag(field, Room::FLAG_NOTRACK, QString("!Track"));
	addFlag(field, Room::FLAG_NOMAGIC, QString("!Magic"));
	addFlag(field, Room::FLAG_NOTELEPORT, QString("!Teleport"));
	addFlag(field, Room::FLAG_DIZZY, QString("Dizzy"));

	/* Room Types */
	field = addField(QString("Room Type"), FIELD_ENUM, &type, true);
	addFlag(field, Room::TYPE_INDOORS, QString("Indoors"));
	addFlag(field, Room::TYPE_COVERED, QString("Covered"));
	addFlag(field, Room::TYPE_FIELD, QString("Field"));
	addFlag(field, Room::TYPE_CITY, QString("City"));
	addFlag(field, Room::TYPE_FARM, QString("Farm"));
	addFlag(field, Room::TYPE_FOREST, QString("Forest"));
	/* Do we need these types? */
	addFlag(field, Room::TYPE_HILL, QString("Hill"));
	/* Do we need these types? */
	addFlag(field, Room::TYPE_MOUNTAIN, QString("Mountain"));
	addFlag(field, Room::TYPE_WATER, QString("Water Surface"));
	addFlag(field, Room::TYPE_UNDERWATER, QString("Underwater"));
	addFlag(field, Room::TYPE_FLYING, QString("Flying"));

	if (!load())
	{
		title = "New Room";
		flags = (1<<Room::FLAG_NOTELEPORT);
	}

	sendMenu();
}

/** Save an OLC room and reload it in the world. */
void RoomOLC::save(void)
{
	QString query;
	QTextOStream qos(&query);
	QSqlQuery q;

	qos << "replace into room (zone, latitude, longitude, elevation, "
			<< "title, flags, type, plrlimit, description, lightlev)" << endl
			<< "values" << endl
			<< "(" << ezone << ", " << elat << ", " << elong << ", " << eelev
			<< ", '" << Logger::escapeString(title) << "'," << endl
			<< flags << ", " << type << ", " << plrlimit << ", " << endl
			<< "'" << Logger::escapeString(description) << "'," << endl
			<< lightlev << ");";
	q.exec(query);

	/* Get a pointer to the specified room and reload it */
	Room *thisroom = Room::findRoom(ezone, elat, elong, eelev);

	if (thisroom)
	{
		thisroom->load();
	} else {
		new Room(ezone, elat, elong, eelev);
	}
}

/** Load a room from the database for OLC */
bool RoomOLC::load(void)
{
	QString query;
	QTextOStream qos(&query);
	QSqlQuery q;

	qos << "select title, flags+0, type+0, plrlimit, lightlev, description"
			<< endl << "from room where" << endl
			<< "zone=" << ezone << " and latitude=" << elat << " and "
			<< "longitude=" << elong << " and elevation=" << eelev << ";";
	if (q.exec(query) && q.numRowsAffected())
	{
		q.next();
		title = q.value(0).toString();
		flags = q.value(1).toInt();
		type = q.value(2).toInt();
		plrlimit = q.value(3).toInt();
		lightlev = q.value(4).toInt();
		description = q.value(5).toString();
		return true;
	} else {
		cout << "Failed Query: " << query << endl;
	}
	return false;
}

namespace commands
{

/* {{{ Move commands */
/** Move command class */
class Move : public Command
{
	public:
		/** Pass through constructor */
		Move(Char *ch) : Command(ch) {}
		/** Run Move command */
		virtual unsigned int run(QString args)
		{
			QString str;
			QTextOStream os(&str);

			Room::directions dir = Roomstringtodirmap[args.upper()];
			if (!doMove(dir))
				return 1;
			return 0;
		}

		/** Preform tasks associated with moving
		 * This consolidates code for all of the move commands into one location.
		 * There might be a couple other places that get a bit of code repetition,
		 * (traveling weave, etc.), but this should cover most instances.
		 */
		bool doMove(Room::directions dir)
		{
			/* First off, we need to see if we can go in 'dir' */
			RoomExit *exit = _ch->getRoom()->getExitFor(dir);
			if (!exit || 
					(exit->isSet(RoomExit::FLAG_HIDDEN) &&
					 exit->isSet(RoomExit::FLAG_CLOSED)))
			{
				QString out;
				QTextOStream os(&out);
				os << endl << "You cannot move in that direction." << endl;
				_ch->sendtochar(out);
				return false;
			}
			
			if (exit->isSet(RoomExit::FLAG_CLOSED))
			{
				QString out;
				QTextOStream os(&out);
				os << endl
					 << "%sender% hit your head on the door trying to walk through it."
					 << endl;
				QString roomtmpl;
				QTextOStream os2(&roomtmpl);
				os2 << endl
						<< "%sender% hits his head while trying to walk through a door."
						<< endl;
				_ch->getRoom()->sendToRoom(_ch, NULL, NULL, out, NULL, roomtmpl);
				return false;
			}

			/** If we get to here, then we have a good exit.
			 * Go ahead and send out a string to the room we are leaving.
			 */
			QString dirString = Roomdirtostringmap[dir].lower();
			QString odirString =Roomdirtostringmap[Roomdiroppositemap[dir]].lower();

			/* FIXME: should loop through group here moving each person - leader
			 * always moves first */
			/* while (groupmembers) */
			{
				/** Send message to start room */
				QString fromtmpl;
				QTextOStream os(&fromtmpl);
				os << endl << "|WYou leave %message%.|x" << endl;
				QString roomtmpl;
				QTextOStream os2(&roomtmpl);
				os2 << endl << "|W%sender% leaves %message%.|x" << endl;
				_ch->getRoom()->sendToRoom(_ch, NULL, dirString,
																		fromtmpl, NULL, roomtmpl);

				_ch->getRoom()->leaveRoom(_ch);
				exit->getDest()->enterRoom(_ch);
				_ch->setRoom(exit->getDest());
				_ch->sendtochar(_ch->getRoom()->displayRoom(_ch, false));

				/** Prepare and send message to new room */
				QString roomtmpl2;
				QTextOStream os3(&roomtmpl2);
				os3 << endl << "|W%sender% arrives from the %message%.|x" << endl;
				exit->getDest()->sendToRoom(_ch, NULL, odirString,
																		NULL, NULL, roomtmpl2);
			}

			return true;
		}
};

/** North command class */
class North : public Move
{
	public:
		/** Pass through constructor */
		North(Char *ch) : Move(ch) {}
		/** Run North command */
		virtual unsigned int run(QString args)
		{
			if (!doMove(Room::DIR_NORTH))
				return 1;
			return 0;
		}
};

/** NorthEast command class */
class NorthEast : public Move
{
	public:
		/** Pass through constructor */
		NorthEast(Char *ch) : Move(ch) {}
		/** Run NorthEast command */
		virtual unsigned int run(QString args)
		{
			if (!doMove(Room::DIR_NORTHEAST))
				return 1;
			return 0;
		}
};

/** East command class */
class East : public Move
{
	public:
		/** Pass through constructor */
		East(Char *ch) : Move(ch) {}
		/** Run East command */
		virtual unsigned int run(QString args)
		{
			if (!doMove(Room::DIR_EAST))
				return 1;
			return 0;
		}
};

/** SouthEast command class */
class SouthEast : public Move
{
	public:
		/** Pass through constructor */
		SouthEast(Char *ch) : Move(ch) {}
		/** Run SouthEast command */
		virtual unsigned int run(QString args)
		{
			if (!doMove(Room::DIR_SOUTHEAST))
				return 1;
			return 0;
		}
};

/** South command class */
class South : public Move
{
	public:
		/** Pass through constructor */
		South(Char *ch) : Move(ch) {}
		/** Run South command */
		virtual unsigned int run(QString args)
		{
			if (!doMove(Room::DIR_SOUTH))
				return 1;
			return 0;
		}
};

/** SouthWest command class */
class SouthWest : public Move
{
	public:
		/** Pass through constructor */
		SouthWest(Char *ch) : Move(ch) {}
		/** Run SouthWest command */
		virtual unsigned int run(QString args)
		{
			if (!doMove(Room::DIR_SOUTHWEST))
				return 1;
			return 0;
		}
};

/** West command class */
class West : public Move
{
	public:
		/** Pass through constructor */
		West(Char *ch) : Move(ch) {}
		/** Run West command */
		virtual unsigned int run(QString args)
		{
			if (!doMove(Room::DIR_WEST))
				return 1;
			return 0;
		}
};

/** NorthWest command class */
class NorthWest : public Move
{
	public:
		/** Pass through constructor */
		NorthWest(Char *ch) : Move(ch) {}
		/** Run NorthWest command */
		virtual unsigned int run(QString args)
		{
			if (!doMove(Room::DIR_NORTHWEST))
				return 1;
			return 0;
		}
};

/** Up command class */
class Up : public Move
{
	public:
		/** Pass through constructor */
		Up(Char *ch) : Move(ch) {}
		/** Run Up command */
		virtual unsigned int run(QString args)
		{
			if (!doMove(Room::DIR_UP))
				return 1;
			return 0;
		}
};

/** Down command class */
class Down : public Move
{
	public:
		/** Pass through constructor */
		Down(Char *ch) : Move(ch) {}
		/** Run Down command */
		virtual unsigned int run(QString args)
		{
			if (!doMove(Room::DIR_DOWN))
				return 1;
			return 0;
		}
};
/* }}} */

/** RoomEdit command class */
class RoomEdit : public Move
{
	public:
		/** Pass through constructor */
		RoomEdit(Char *ch) : Move(ch) {}
		/** Run RoomEdit command */
		virtual unsigned int run(QString args)
		{
			QString loc = args.replace(QRegExp("("), " ");
			loc = loc.replace(QRegExp(")"), " ");
			loc = loc.replace(QRegExp(","), " ");

			int zone,lat,longi,elev;
			zone = loc.section(' ', 0,0).toInt();
			lat = loc.section(' ', 1,1).toInt();
			longi = loc.section(' ', 2,2).toInt();
			elev = loc.section(' ', 3,3).toInt();
			Parser *_oldparser = _ch->getDesc()->parser();
			_ch->getDesc()->setParser(
							new RoomOLC(_ch, _ch->getDesc(), _oldparser,
													zone, lat, longi, elev), false);
			return 0;
		}
};

}; /* end commands namespace */

/** Command Factory for cmd.cpp */
class Room_CPP_CommandFactory : public CommandFactory
{
	public:
		/** Register our commands */
		Room_CPP_CommandFactory(void)
			: CommandFactory()
		{
			maincmdtree->addcmd("north", this, 2);
			maincmdtree->addcmd("n", this, 2);
			maincmdtree->addcmd("northeast", this, 3);
			maincmdtree->addcmd("ne", this, 3);
			maincmdtree->addcmd("east", this, 4);
			maincmdtree->addcmd("e", this, 4);
			maincmdtree->addcmd("south", this, 6);
			maincmdtree->addcmd("s", this, 6);
			maincmdtree->addcmd("southeast", this, 5);
			maincmdtree->addcmd("se", this, 5);
			maincmdtree->addcmd("southwest", this, 7);
			maincmdtree->addcmd("sw", this, 7);
			maincmdtree->addcmd("west", this, 8);
			maincmdtree->addcmd("w", this, 8);
			maincmdtree->addcmd("northwest", this, 9);
			maincmdtree->addcmd("nw", this, 9);
			maincmdtree->addcmd("up", this, 10);
			maincmdtree->addcmd("u", this, 10);
			maincmdtree->addcmd("down", this, 11);
			maincmdtree->addcmd("d", this, 11);
			olccmdtree->addcmd("room", this, 12);
		}

		/** Handle command object creations */
		virtual Command *create(unsigned int id, Char *ch)
		{
			switch (id)
			{
				case 1:
					return new koalamud::commands::Move(ch);
				case 2:
					return new koalamud::commands::North(ch);
				case 3:
					return new koalamud::commands::NorthEast(ch);
				case 4:
					return new koalamud::commands::East(ch);
				case 5:
					return new koalamud::commands::SouthEast(ch);
				case 6:
					return new koalamud::commands::South(ch);
				case 7:
					return new koalamud::commands::SouthWest(ch);
				case 8:
					return new koalamud::commands::West(ch);
				case 9:
					return new koalamud::commands::NorthWest(ch);
				case 10:
					return new koalamud::commands::Up(ch);
				case 11:
					return new koalamud::commands::Down(ch);
				case 12:
					return new koalamud::commands::RoomEdit(ch);
			}
			return NULL;
		}
};

/** Command factory for room.cpp module.  */
Room_CPP_CommandFactory Room_CPP_CommandFactoryInstance;

}; /* end koalamud namespace */
