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
};

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

	protected: /* Internal typedefs */
		/** Command queue object */
		typedef struct {
			Command *cmd; /**< Pointer to the command */
			QString args; /**< Command arguments */
			/** Operator new overload */
			void * operator new(size_t obj_size)
				{ return koalamud::PoolAllocator::alloc(obj_size); }
			/** Operator delete overload */
			void operator delete(void *ptr)
				{ koalamud::PoolAllocator::free(ptr); }
		} cmdqueueitem;

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
		virtual void queueCommand(Command *cmd, QString args);
		/** Return true if this descriptor is disconnecting */
		virtual bool isDisconnecting(void) {return _disconnecting; }
		/** Return a pointer to the room we are in */
		Room *getRoom(void) const { return _inroom; }
		/** Update the room we are in
		 * Assume that calling function handles the player lists for the room pair
		 */
		void setRoom(Room *newroom) { _inroom = newroom; }
		QString languageMorph(QString langid, QString msg, bool spoken = false);
		

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
		virtual void sendPrompt(void) { sendtochar("prompt>");}
		/** Reset disconnecting status */
		virtual void setdisconnect(bool dis = true) { _disconnecting = dis;}
		/** A channel has just been deleted */
		virtual void channeldeleted(Channel *chan) { if (chan) return;}
		/** Handle closing descriptor */
		virtual void descriptorClosed(void) { _desc = NULL;}

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
		/** Pointer to attached descriptor */
		ParseDescriptor *_desc;
		/** Disconnecting flag */
		bool _disconnecting;
		/** Pointer to room we are in */
		Room *_inroom;

		/** Our command queue */
		QPtrQueue<cmdqueueitem> cmdqueue;
		/** Lock for command queue */
		ZThread::FastRecursiveMutex cmdqueuelock;
		/** Is our command task running */
		bool cmdtaskrunning;

	protected: /* Internal usage classes */
		/** Command executor task */
		class cmdexectask : public ZThread::Runnable
		{
			public:
				/** Create a command executor task */
				cmdexectask(Char *ch) : _ch(ch) {}
				/** Destroy a command executor task */
				virtual ~cmdexectask(void) {}
				virtual void run(void);
			protected:
				/** Pointer to the char we are executing commands on */
				Char *_ch;
		};
		/** Make sure that command executor can get to everything it needs */
		friend class cmdexectask;
};

}; /* end koalamud namespace */

#endif  // KOALA_CHAR_HXX
