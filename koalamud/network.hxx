/***************************************************************\
*                        KoalaMud Gen 2                         *
*    Copyright (c) 2002 First Step Internet Services, Inc.      *
*                     All Rights Reserved                       *
*        Distributed under the terms of the FSI License         *
* See file LICENSE in the root of this package for information  *
\***************************************************************/
/***************************************************************\
*	Module:   CORE/NETWORK
* Description:
*       Network Support code - server class and connection
*       inherited by K_PlayerChar
* Classes:
*     KoalaServer
*     KoalaDescriptor
\***************************************************************/

#ifndef KOALA_NETWORK_HXX
#define KOALA_NETWORK_HXX "%A%"

#include <qserversocket.h>
#include <qsocket.h>
#include <qlistview.h>
#include <zthread/FastRecursiveMutex.h>

/* Predefine classes */
namespace koalamud {
class ParseDescriptor;
};

#include "parser.hxx"

/*  All of the network code needs to be rebuilt in the koalamud namespace.. */
namespace koalamud {

/** Network listener class
 * This class listens to a specified port for incoming connections and creates
 * appropriate descriptor objects to match listener type.
 */
class Listener : public QServerSocket
{
	public:
		/** Listener types */
		typedef enum {
			GAMESERVER, /**< Listener is a game player listener */
		} porttype_t;

	public:
   Listener(unsigned int port, porttype_t = GAMESERVER);
   virtual ~Listener();

  /** Handle a newly accepted connection and bring it into the game world */
  virtual void newConnection(int socket);

protected:
	/** Pointer to our status item in the gui */
	QListViewItem *ListenStatusItem;
	/** Type of Listener */
	porttype_t _type;
	/** Port we are listening on */
	unsigned int _port;
		
};

/** Descriptors base class
 * There will be several types of descriptors eventually, but all of them
 * should inherit this class.  This will allow us to connect any descriptor to
 * a character and be able to send data back out over the descriptor.  Some
 * descriptor types will provide additional functionality depending on their
 * purpose.
 * @note  The base descriptor class only provides send functionality.
 */
class Descriptor : public QSocket
{
	Q_OBJECT

	public:
		Descriptor(int sock);
		/** Destroy a descriptor */
		virtual ~Descriptor(void) {};

		/** Operator new overload */
		void * operator new(size_t obj_size)
			{ return koalamud::PoolAllocator::alloc(obj_size); }
		/** Operator delete overload */
		void operator delete(void *ptr)
			{ koalamud::PoolAllocator::free(ptr); }

	public:
		virtual void send(QString sendthis);
		virtual bool event(QEvent *event);
		void notifyOutput(Char *ch);

	protected:
		/** True if an event is posted */
		bool outputEventPosted;
		/** Lock for output event flag */
		ZThread::FastRecursiveMutex outputEventLock;

	private slots:
		virtual void readClient(void);
		virtual void closed(void);
};

/** Descriptor with hooks to parser
 * This version of the Descriptor includes hooks to parser objects.  It will
 * read data and pass it to a parser class.  The parser provides the
 * connection between descriptors and character objects.  While there are some
 * linkings directly between descriptors and characters, it is mostly a one
 * way connection.
 * This class introduces setParser() and parser() in order to change the
 * attached parser.
 */
class ParseDescriptor : public Descriptor
{
	Q_OBJECT

	public:
		ParseDescriptor(int sock, Parser *parser = NULL);
		virtual ~ParseDescriptor(void);

	public:
		void setParser(Parser *newparse, bool del=true);
		/** Return a pointer to the currently attached parser */
		Parser *parser(void) { return _parse; }

	private slots:
		virtual void readClient(void);

	protected:
		/** Pointer to the attached parser */
		Parser *_parse;
};

}; /* end koalamud namespace */

#endif  // KOALA_NETWORK_HXX
