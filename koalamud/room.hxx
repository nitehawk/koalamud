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

#include <qobject.h>

#include "autoptr.hxx"

/** KoalaMud Room.
 *
 * @author Matthew Schlegel <nitehawk@koalamud.org>
 *
 * Representation of an in game room.
 *
 * @warning Still a lot to be cleaned up here, both in implementation and
 * design.  A lot of work will be needed to ensure that wilderness rooms in
 * particular do not end up consuming all available memory.
 */
class K_Room : public QObject, public virtual KRefObj
{
	Q_OBJECT

	public:
		/** Constant default latitude used to identify requests for a new OLC
		 * room. */
		static const int defnewlat = 2000000;
		/** Constant default longitude used to identify requests for a new OLC
		 * room. */
		static const int defnewlong = 2000000;
		/** Constant default elevation used to identify requests for a new OLC
		 * room. */
		static const int defnewele = 2000000;
		/** Smart Pointer type */
		typedef KRefPtr<K_Room> autoptr;
		/** Room Flags.
		 * The list of all flags possible to be set on a room */
		typedef enum {
				/** No flags set */
				FLAG_NOFLAGS = 0x0,
				/** Room has a level restriction, can be minimum or maximum or both.*/
				FLAG_LEVELRESTRICT = 0x1,
				/** Room provides fast regen. */
				FLAG_REGEN = 0x2,
				/** Room is dark - outdoor rooms are dark at night. */
				FLAG_DARK = 0x4,
				/** Room is a deathtrap. */
				FLAG_DEATHTRAP = 0x8,
				/** No NPC's allowed in room. */
				FLAG_NONPC = 0x10,
				/** Room is a safe spot, no fighting. */
				FLAG_SAFE = 0x11,
				/** Room is a save spot.  Players not logged in for an extended
				 * period are drawn to savespot rooms */
				FLAG_SAVESPOT = 0x12,
				/** Room is a recall location. */
				FLAG_RECALLSPOT = 0x14,
				/** Track command is blocked at this room. */
				FLAG_NOTRACK = 0x18,
				/** No magic allowed in this room. */
				FLAG_NOMAGIC = 0x20,
				/** Teleport command can choose this room as a target. */
				FLAG_TELEPORT = 0x21,
				/** Room is part of the wilderness.  Changes saving semantics and some
				 * aspects of room loading and lifetimes. */
				FLAG_WILDERNESS = 0x22
				} flags_t;
		/** Room Types
		 * @notes Generally speaking, Room types INDOORS, COVERED, FIELD, and CITY
		 * will be found inside of hand built areas, and the remaining types will
		 * be found in wilderness areas.  
		 * @todo  Add Road type(s)
		 */
		typedef enum {TYPE_INDOORS = 0, /**< Room in Inside of a building */
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

	public:
		K_Room(int lat=defnewlat, int longi=defnewlong, int ele=defnewele,
						unsigned int zone=1);
		K_Room(QString, QString, flags_t rflags=FLAG_NOFLAGS,
						type_t rtype=TYPE_INDOORS);
		virtual ~K_Room();

		virtual bool load(void);
		virtual bool save(void);

	protected:
		QString _title; /**< Room Title */
		QString _description;  /**<  Room description */
		bool _virtual;	/**< Flag to denote a virtual, unsaved room */
		int _lat; /**< Room latitude coordinate */
		int _long; /**< Room longitude coordinate */
		int _elev; /**< Room Elevation coordinate */
		/** Maximum number of players in room, 0 is unlimited */
		unsigned int _plrlimit;
		unsigned int _lvlmin; /**< Minimum level to enter, 0 is no minimum */
		unsigned int _lvlmax; /**< Maximum level to enter, 0 is no minimum */
		flags_t _flags;  /**< Room flags */
		type_t _rtype;  /**< Room type */
		/** Zone identifier, used for various purposes.
		 * @todo Possibly add a pointer to the zone object and remove this version
		 * in favor of the pointer.
		 */
		unsigned int _zoneid;
		/** Flag to change the behaviour of save to handle a room already in the
		 * database. */
		bool _indatabase;

};

#endif //  KOALA_ROOM_HXX
