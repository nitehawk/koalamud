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

#ifndef KOALA_ROOM_HXX
#define KOALA_ROOM_HXX "%A%"

#include <qstring.h>
#include <qdict.h>
#include <bitset>

#include <zthread/FastRecursiveMutex.h>

#include "memory.hxx"

/* Predefine classes */
namespace koalamud {
class Room;
};

#include "char.hxx"

namespace koalamud {

/** Room Exit
 * This provides the connection between rooms.  This is a one way connection
 * *only*.  Two way connections use an exit each direction.
 * @note The Room class is responsible for saving exits to the database where
 * needed.  This saves us the burden of needing virtual flags and similar.
 */
class RoomExit
{
	public:
		/** Flags for this exit */
		typedef enum {
			/** No flags - direct link */
			FLAG_NONE = 0,
			/** Exit is a door */
			FLAG_DOOR,
			/** Door is lockable */
			FLAG_LOCKABLE,
			/** Door is locked */
			FLAG_LOCKED,
			/** Door is lockpick proof */
			FLAG_PICKPROOF,
			/** Door is closed */
			FLAG_CLOSED,
			/** Door is magically locked */
			FLAG_MAGICLOCK,
			/** Door/Exit is hidden */
			FLAG_HIDDEN,
			/** Exit is trapped - search source room for affects */
			FLAG_TRAPPED,
			/** Lock is magic proof (no magicunlock) */
			FLAG_MAGICPROOF,
			/** Door is bash proof - no muscling our way in */
			FLAG_BASHPROOF,
			/** This must be the last flag */
			FLAG_ENDLIST
		} flag_t;
		
	public:
		/** Initialize a room exit */
		RoomExit(Room *destroom, flag_t exitflags, QString name=NULL,
							unsigned int keyobj=0)
			: _name(name), dest(destroom), flags(exitflags), keynum(keyobj)
		{}
		
	public: /* Operators */
		/** Operator new overload */
		void * operator new(size_t obj_size)
			{ return koalamud::PoolAllocator::alloc(obj_size); }
		/** Operator delete overload */
		void operator delete(void *ptr)
			{ koalamud::PoolAllocator::free(ptr); }

	public:
		/** Is the exit visible */
		bool isVisible(void) const { return (!(flags[FLAG_HIDDEN]));}
		/** Return true if a flag is set */
		bool isSet(flag_t flag) const { return (flags[flag]);}
		/** Return pointer to destination room or NULL if the door is closed */
		Room *getDest(void) { if (flags[FLAG_CLOSED]) return NULL; return dest;}
		/** Toggle flag and return new state */
		bool toggleFlag(flag_t flag)
			{ flags[flag] = !flags[flag];
				return flags[flag];}

	protected:
		QString _name; /**< Exit name (used in the case of doors */
		Room *dest; /**< Other end of this link */
		bitset<FLAG_ENDLIST> flags; /**< Exit flags */
		unsigned int keynum; /**< Object ID that is key */
};

/** KoalaMud Room.
 *
 * @author Matthew Schlegel <nitehawk@koalamud.org>
 *
 * Representation of an in game room.
 *
 * This may be subclassed to make some usages easier as well.
 */
class Room
{
	public:
		/** Room Flags.
		 * The list of all flags possible to be set on a room
		 * This should match the order used in the database */
		typedef enum {
				/** No flags set */
				FLAG_NOFLAGS = 0,
				/** Room has a level restriction, can be minimum or maximum or both.*/
				FLAG_LEVELRESTRICT,
				/** Room provides fast regen. */
				FLAG_REGEN,
				/** Room is dark - outdoor rooms are dark at night. */
				FLAG_DARK,
				/** Room is a deathtrap. */
				FLAG_DEATHTRAP,
				/** No NPC's allowed in room. */
				FLAG_NONPC,
				/** Room is a safe spot, no fighting. */
				FLAG_SAFE,
				/** Room is a save spot.  Players not logged in for an extended
				 * period are drawn to savespot rooms */
				FLAG_SAVESPOT,
				/** Room is a recall location. */
				FLAG_RECALLSPOT,
				/** Track command is blocked at this room. */
				FLAG_NOTRACK,
				/** No magic allowed in this room. */
				FLAG_NOMAGIC,
				/** Teleport command can choose this room as a target. */
				FLAG_TELEPORT,
				/** Last flag - used for sizing bitset */
				FLAG_ENDLIST
				} flags_t;
		/** Room Types
		 * @notes Generally speaking, Room types INDOORS, COVERED, FIELD, and CITY
		 * will be found inside of hand built areas, and the remaining types will
		 * be found in wilderness areas.  
		 * @todo  Add Road type(s)
		 */
		typedef enum {
			TYPE_INDOORS = 0, /**< Room in Inside of a building */
			TYPE_COVERED,	/**< Room is Covered in some form */
			TYPE_FIELD, /**< Room is a field - perhaps a park */
			TYPE_CITY, /**< Room is a city, but not one of the above */
			TYPE_FARM, /**< Room is part of a farm */
			TYPE_FOREST, /**< Room is part of a forest */
			TYPE_HILL,	/**< Room is part of a hill */
			TYPE_MOUNTAIN, /**< Room is part of a mountain */
			TYPE_WATER, /**< Room is at the surface of water */
			TYPE_UNDERWATER, /**<< Room is underwater */
			TYPE_FLYING /**< Room is in the air */
			} type_t;

		/** Map directions to their position in the exit pointer array
		 * @note DIR_DOWN must be the last element in the list */
		typedef enum {
			DIR_NORTH = 0,
			DIR_NORTHEAST,
			DIR_EAST,
			DIR_SOUTHEAST,
			DIR_SOUTH,
			DIR_SOUTHWEST,
			DIR_WEST,
			DIR_NORTHWEST,
			DIR_UP,
			DIR_DOWN,
		} directions;

	public:
		Room(int zone, int lat, int longi, int ele);
		Room(QString, QString, QString, QString, QString,
						int, int, int, int,
						flags_t rflags=FLAG_NOFLAGS, type_t rtype=TYPE_INDOORS,
						unsigned int plrlimit=0);
		virtual ~Room();

		virtual bool load(void);
		virtual bool save(void);

	public: /* Operators */
		/** Operator new overload */
		void * operator new(size_t obj_size)
			{ return koalamud::PoolAllocator::alloc(obj_size); }
		/** Operator delete overload */
		void operator delete(void *ptr)
			{ koalamud::PoolAllocator::free(ptr); }

	public:
		/** Call to add a character to a room */
		void enterRoom(Char *ch)
			{ lock.acquire();
				charsinroom.append(ch);
				lock.release(); }
		/** Call to remove a character from a room */
		void leaveRoom(Char *ch)
			{ lock.acquire();
				charsinroom.remove(ch);
				lock.release(); }
		void moveRoom(int, int, int, int);
		QString displayRoom(Char *ch, bool brief=false);
		void sendToRoom(Char *from, Char *to, QString msg, QString fromtmpl,
						QString totmpl, QString roomtmpl);

	public:  /* Static functions */
		static Room *findRoom(int zone, int lat, int longi, int elev);
		/** Return a string representing a room location */
		static QString getRef(int zone, int lat, int longi, int elev)
			{ QString ref; QTextOStream os(&ref);
				os << "(" << zone << "," << lat << "," << longi << "," << elev << ")";
				return ref; }

	protected:
		QString _title; /**< Room Title */
		QString _description;  /**<  Room description */
		QString _smell; /**< Smell description */
		QString _sound; /**< Sound description */
		QString _soundfile; /**< filename for sound using mud sound protocol */
		bool _virtual;	/**< Flag to denote a virtual, unsaved room */
		int _zone; /**< Zone Coordinate */
		int _lat; /**< Room latitude coordinate */
		int _long; /**< Room longitude coordinate */
		int _elev; /**< Room Elevation coordinate */
		/** Maximum number of players in room, 0 is unlimited */
		unsigned int _plrlimit;
		bitset<FLAG_ENDLIST> _flags;  /**< Room flags */
		type_t _rtype;  /**< Room type */
		ZThread::FastRecursiveMutex lock; /**< Lock for room */
		/** Pointers to exit objects to connect to neighbor rooms */
		RoomExit *exits[DIR_DOWN+1];
		/** List of chars in room */
		QPtrList<Char> charsinroom;
};

#ifdef KOALA_ROOM_CXX
QDict<Room> RoomMap(8388607);
#else
extern QDict<Room> RoomMap;
#endif

}; /* end koalamud namespace */

#endif //  KOALA_ROOM_HXX
