/***************************************************************\
*                        KoalaMud Gen 2                         *
*    Copyright (c) 2002 First Step Internet Services, Inc.      *
*                     All Rights Reserved                       *
*        Distributed under the terms of the FSI License         *
* See file LICENSE in the root of this package for information  *
\***************************************************************/
/***************************************************************\
*	Module: CORE
* Description:
* Classes:
\***************************************************************/

#ifndef KOALA_MAIN_HXX
#define KOALA_MAIN_HXX "%A%"

#include <zthread/PoolExecutor.h>
#include <qapplication.h>
#include "network.hxx"
#include "koalastatus.h"
#include "memory.hxx"
#include "database.hxx"
#include "exception.hxx"

namespace koalamud {
	/* Predeclare socket */
	class Socket;

/** Main server class
 * This class handles all of the setup and execution of the game engine.  It
 * is responsible for initializing all of the subsystems and providing a
 * proxy layer for some of the other subsystems to communicate with each
 * other.
 */
class MainServer 
{
	protected: /* constants */
		/** Minimum number of threads to create for ZThread threadpool. */
		static const unsigned int threadpoolmin = 1;
		/** Maximum number of threads to create for ZThread threadpool. */
		static const unsigned int threadpoolmax = 2;

	protected: /* internal data */
		/** Pointer to database management */
		koalamud::Database *_kmdb;
		/** Pointer to the Qt Application object */
		QApplication *_app;
		/** Pointer to our task executor */
		ZThread::Executor *_executor;
		/** Gui status */
		bool _guiactive;
		/** Pointer to our status window */
		KoalaStatus *_statwin;
		/** Should we run in the background */
		bool _background;
		/** Execution Profile */
		QString _profile;
		/** Shutdown Flag - true if we are shutting down */
		bool shutdown;
		/** Socket list */
		QPtrList<Socket> socklist;

	public: /* Base system execution functions */
		MainServer(int argc, char **argv) throw(koalaexception);
		~MainServer(void);

		void run(void);

	public: /* property extraction functions */
		/** Return a pointer to our pool executor */
		ZThread::Executor *executor(void) { return _executor; }
		/** Return a pointer to our QAppliction object */
		QApplication *app(void) { return _app; }
		/** Return a pointer to our database object */
		koalamud::Database *db(void) { return _kmdb; }
		/** Return true if we are using the GUI */
		bool usegui(void) { return _guiactive; }
		/** Return pointer to status window */
		KoalaStatus *statwin(void) { return _statwin; }
		/** Return true if we are detached from the console */
		bool isdetached(void) { return _background; }
		void Shutdown(void) { shutdown = true; }
		/** Add a socket to the socket list */
		void addSocktoList(Socket *sock) { socklist.append(sock);}
		/** Remove a socket from the socket list */
		void removeSockfromList(Socket *sock) { socklist.remove(sock);}
		
	protected: /* Internal utility functions */
		void parseargs(int argc, char **argv) throw (koalaexception);
		void daemonize(void) throw (koalamud::exceptions::daemonize);

	public: /* public utility functions */
		QString versionstring(void);
		QString usage(void);
};
	
}; /* end koalamud namespace */

#ifdef KOALA_MAIN_CXX
koalamud::MainServer *srv;
#else
extern koalamud::MainServer *srv;
#endif // KOALA_MAIN_CXX


#endif  // KOALA_MAIN_HXX
