/***************************************************************\
*                        KoalaMud Gen 2                         *
*    Copyright (c) 2002 First Step Internet Services, Inc.      *
*                     All Rights Reserved                       *
*        Distributed under the terms of the FSI License         *
* See file LICENSE in the root of this package for information  *
\***************************************************************/
/***************************************************************\
*	Module: CHAR
* Description:  Character baseclass used for both NPC and PC characters
* Classes: Char
\***************************************************************/

#ifndef KOALA_CHAR_HXX
#define KOALA_CHAR_HXX "%A%"

#include <qobject.h>
#include <qptrqueue.h>

#include <zthread/FastRecursiveMutex.h>
#include <zthread/Guard.h>
#include <zthread/Thread.h>

#include "memory.hxx"

/* predefine ourself */
namespace koalamud {
	class Char;
	class Language;
};

#include "skill.hxx"
//#include "language.hxx"
#include "network.hxx"
#include "cmd.hxx"
#include "comm.hxx"
#include "room.hxx"

namespace koalamud {
/** Character abstract base class
 * This class handles the majority of functions associated with characters,
 * whether PC or NPC.
 */
class Char : public QObject
{
	Q_OBJECT

	public: // Constructors/destructors
		Char(QString name = NULL, ParseDescriptor *desc=NULL);
		/** Empty virtual destructor to ensure cleanup happens correctly */
		virtual ~Char(void);

	public:
		/** Update name */
		virtual void setName(QString name)
			{ _name = name; }
		virtual QString getName(Char *pair=NULL);
		virtual QString getShortDesc(Char *pair=NULL);
		virtual void setDesc(ParseDescriptor *desc);
		/** Return pointer to connected descriptor */
		virtual ParseDescriptor *getDesc(void) { return _desc; }
		virtual bool visibleTo(Char *pair);
		/** Is this a player char
		 * Return true if this is a player character, false otherwise */
		virtual bool isPC(void) { return false;}
		/** Return true if this descriptor is disconnecting */
		virtual bool isDisconnecting(void) {return _disconnecting; }
		/** Return a pointer to the room we are in */
		Room *getRoom(void) const { return _inroom; }
		/** Update the room we are in
		 * Assume that calling function handles the player lists for the room pair
		 */
		void setRoom(Room *newroom) { _inroom = newroom; }
		QString languageMorph(QString langid, QString msg, bool spoken = false);
		/** Should we show a compass in room descriptions */
		bool showCompass(void) const { return true; }
		/** Return true if the character is an immortal
		 * This determines if the immcommandtree is searched for commands */
		virtual bool isImmortal(void) const {return true;}
		

	public: /* Pure virtuals */
		/** Load character from database
		 * This operation is different for PC's and NPC's
		 */
		virtual bool load(void) = 0;
		/** Save character to database
		 * This operation is different for PC's and NPC's
		 */
		virtual bool save(void) = 0;

	public slots: /* Qt Slots */
		virtual void channelsendtochar(Char *from, QString, QString, QString);
		/** Send data to character */
		virtual bool sendtochar(QString data);
		/** Send prompt to descriptor */
		virtual void sendPrompt(void) { sendtochar("prompt>\377\371");}
		virtual void setdisconnect(bool dis = true);
		/** A channel has just been deleted */
		virtual void channeldeleted(Channel *chan) { if (chan) return;}
		/** Handle closing descriptor */
		virtual void descriptorClosed(void) { _desc = NULL;}
		virtual int getKnow(QString id);
		virtual bool setSkillLevel(QString id, int level);
		/** Get an iterator for the skills list */
		virtual QDictIterator<SkillRecord> getSkrecIter(void)
			{ QDictIterator<SkillRecord> skcur(skills); return skcur; }

	public: /* Operators */
		/** Operator new overload */
		void * operator new(size_t obj_size)
			{ return koalamud::PoolAllocator::alloc(obj_size); }
		/** Operator delete overload */
		void operator delete(void *ptr)
			{ koalamud::PoolAllocator::free(ptr); }

	protected:
		/** Character name */
		QString _name;
		/** Character last name */
		QString _lname;
		/** Pointer to attached descriptor */
		ParseDescriptor *_desc;
		/** Disconnecting flag */
		bool _disconnecting;
		/** Pointer to room we are in */
		Room *_inroom;
		/** Our primary language */
		Language *priLanguage;
		/** Our skill records */
		QDict<SkillRecord> skills;
};

}; /* end koalamud namespace */

#endif  // KOALA_CHAR_HXX
