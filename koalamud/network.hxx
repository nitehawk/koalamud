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

#include <qlistview.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <zthread/FastRecursiveMutex.h>

#include "buffer.hxx"

/* Predefine classes */
namespace koalamud {
class ParseDescriptor;
};

#include "parser.hxx"

/*  All of the network code needs to be rebuilt in the koalamud namespace.. */
namespace koalamud {

/** Socket class
 * This provides the most basic interface information for network sockets
 */
class Socket : public QObject
{
	Q_OBJECT

	public:
		Socket(int sock = 0);
		virtual ~Socket(void);

		/** This is called by the main socket loop for reads. */
		virtual void dispatchRead(void) = 0;
		/** This is called by the main socket loop for writes. */
		virtual void doWrite(void) = 0;

		/** Return true if there is data waiting in the output buffer
		 * This will affect whether a socket is added to the write socket list */
		virtual bool isDataPending(void) { return false; }
		/** Get the underlying socket descriptor */
		int getSock(void) const { return _sock; }
		/** Mark a socket for closing.
		 * Actual close and delete will be handled by a dispatchWrite function to
		 * allow all of the out buffer to clear first.
		 */
		void markClose(void) { _closeme = true; }

		/** Set all the appropriate socket options on newly accepted sockets */
	protected:
		/** Socket descriptor */
		int _sock;
		/** Should we close when the output buffer empties */
		bool _closeme;
};

/** Network listener class
 * This class listens to a specified port for incoming connections and creates
 * appropriate descriptor objects to match listener type.
 */
class Listener : public Socket
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
		/** This is called by the main socket loop for reads. */
		virtual void dispatchRead(void);
		/** This is called by the main socket loop for writes. */
		virtual void doWrite(void) {}

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
class Descriptor : public Socket
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
		virtual void dispatchRead(void);
		virtual void doWrite(void);
		virtual bool isDataPending(void);

		/** Set color flag */
		void setColor(bool flag) { sendcolor = flag;}
		/** Get color flag */
		bool getColor(void) { return sendcolor;}

	protected:
		/** True if we want to send color on the link */
		bool sendcolor;
		/** Input buffer */
		Buffer inBuffer;
		/** Output buffer */
		Buffer outBuffer;

	private slots:
		virtual void readClient(void);
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
