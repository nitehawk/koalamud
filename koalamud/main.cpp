/***************************************************************\
*                        KoalaMud Gen 2                         *
*    Copyright (c) 2002 First Step Internet Services, Inc.      *
*                     All Rights Reserved                       *
*        Distributed under the terms of the FSI License         *
* See file LICENSE in the root of this package for information  *
\***************************************************************/
/***************************************************************\
*	Module: CORE/Main
* Description:  Server startup
* Classes:
\***************************************************************/

#define KOALA_MAIN_CXX "%A%"

#include <qapplication.h>
#include <qstatusbar.h>

#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "main.hxx"
#include "logging.hxx"
#include "exception.hxx"
#include "database.hxx"
#include "network.hxx"
#include "koalastatus.h"
#include "cmdtree.hxx"
#include "language.hxx"
#include "room.hxx"

namespace koalamud {

/** Main server constructor
 * Here we initialize and prepare the various subsystems and load all of the
 * server configuration information from the database.
 */
MainServer::MainServer( int argc, char **argv ) throw(koalaexception)
	: _executor(NULL), _guiactive(false), _background(false),
		_profile("default"), shutdown(false)
{
	/* Initialize our task pool executor */
	_executor = new ZThread::PoolExecutor<ZThread::FastMutex>(threadpoolmin, threadpoolmax);

	/* Call to process arguments here */
	parseargs(argc, argv);

	/* Daemonizing code here */
	if (_background)
		daemonize();

	/* create our application object */
  _app = new QApplication( argc, argv, _guiactive );

	if (_guiactive) {
     _statwin = new KoalaStatus();
     _statwin->statusBar()->message("booting");
     _statwin->show();
     _app->setMainWidget(_statwin);
   }

	/* Start database */
	_kmdb = new koalamud::Database;
	if (!_kmdb->isonline())
	{
		/* Houston, we have a problem! */
		cerr << "FATAL:  Unable to connect to SQL server!" << endl;
		return;
	}

	Language::loadLanguages();
	Room::loadWorldRooms();
}

/** Start everything running
 * @todo Port information should be loaded from the database
 */
void MainServer::run(void)
{
	/* This will be replaced with exception handling */
	if (!_kmdb || !_kmdb->isonline())
		return;

	Logger::msg("Starting listeners", Logger::LOG_NOTICE);

	/* Start listeners from database */
	{
		QValueList<int> portlist = _kmdb->getListenPorts(_profile);
		QValueList<int>::iterator cur;
		for (cur = portlist.begin(); cur != portlist.end(); ++cur)
		{
			new koalamud::Listener(*cur);
		}
	}

	/* Update status bar */
	if (_guiactive) {
     _statwin->statusBar()->message("online");
   }

	/* Main game loop.  Process Qt events and do our own socket handling for all
	 * descriptors except the MySQL stuff */
	{
		fd_set insockets, outsockets, errsockets;
		struct timeval waitcycle;
		int descriptorcount = 0;
		int maxfd = 0;
		QPtrListIterator<Socket> desc(socklist);
		int selectreturn;
		while (!shutdown)
		{
			maxfd = descriptorcount = 0;
			FD_ZERO(&insockets);
			FD_ZERO(&outsockets);
			FD_ZERO(&errsockets);

			/* Loop through the descriptor list and add sockets to approprate lists
			 */
			for (desc.toFirst(); desc.current(); ++desc)
			{
				/* Everyone gets added to in and error. */
				FD_SET((*desc)->getSock(), &errsockets);
				FD_SET((*desc)->getSock(), &insockets);

				if ((*desc)->isDataPending())
				{
					FD_SET((*desc)->getSock(), &outsockets);
				}

				descriptorcount++;
				if (maxfd < (*desc)->getSock())
					maxfd = (*desc)->getSock();
			}

			/* Setup wait timer struct */
			waitcycle.tv_sec = 0;
			waitcycle.tv_usec = 50; /* 50 microseconds per loop */

			if ((selectreturn = select(maxfd+1, &insockets, &outsockets,
																 &errsockets, &waitcycle)) < 0)
			{
				if (errno == EINTR)
				{
					continue;
				}
				return;
			}

			/* Find activated descriptors */
			if (selectreturn > 0)
			{
				for (desc.toFirst(); desc.current(); ++desc)
				{
					/* Check write */
					if (FD_ISSET((*desc)->getSock(), &outsockets))
					{
						(*desc)->doWrite();
					}
					
					/* Check read */
					if (FD_ISSET((*desc)->getSock(), &insockets))
					{
						(*desc)->dispatchRead();
					}

					/* Check error */
					if (FD_ISSET((*desc)->getSock(), &errsockets))
					{
						/* Close any sockets in error */
						delete (*desc);
					}
				}
			}

			/* Process Qt Events */
			_app->processEvents(50);
		}
	}
}

/** Shut down game server */
MainServer::~MainServer(void)
{
	if (_executor) {
		_executor->cancel();
	}

	delete _kmdb;
	delete _app;
}

/** Parse command line arguments */
void MainServer::parseargs(int argc, char **argv) throw (koalaexception)
{
	char opt = EOF; // Current option

	opterr = 0;

	const char optlist[] = "hfbgGr:p:u:s:d:";

	while ((opt = getopt(argc, argv, optlist)) != -1)
	{
		switch (opt)
		{
			case 'f':
				_background = false;
				break;
			case 'b':
				_background = true;
				break;
			case 'g':
				_guiactive = false;
				break;
			case 'G':
				_guiactive = true;
				break;
			case 'r':
				_profile = optarg;
				break;
			case 'u': /* dbuser */
				break;
			case 'p': /* dbpass */
				break;
			case 's': /* dbserver */
				break;
			case 'd': /* dbname */
				break;
			case ':':
				cout << "Missing argument to " << argv[optind] << endl;
			case 'h':
				cout << usage();
				throw koalaexception();  // we will probably want to make this more specific later
			default:
				/* We don't know this argument, but it may be because it is an X
				 * argument and will be handled by QApplication later */
				break;
		}
	}
	Logger::setProfile(_profile);
}

/** Return current version string */
QString MainServer::versionstring(void)
{
	return "KoalaMud Gen2 v0.3.6a";
}

/** Return server usage string */
QString MainServer::usage(void)
{
	QString outstr;
	QTextOStream os(&outstr);
	os << versionstring() << endl << endl <<
"Usage:" << endl <<
"  koalamud [-h]" << endl << endl << 
"  -h         This help screen" << endl <<
"  -f         Run server in foreground (default)" << endl <<
"  -b         Run server in background" << endl <<
"  -g         disable GUI (default)" << endl <<
"  -G         enable GUI" << endl <<
"  -r					execution profile" << endl;

	return outstr;
}

/** Drop server into background for execution */
void MainServer::daemonize(void) throw (koalamud::exceptions::daemonize)
{
	int status = -1;

	/* Stage 1, spawn into child process and close handles */
	status = fork();
	switch (status)
	{
		case -1:
			perror("daemonize()::fork() - 1");
			throw koalamud::exceptions::forkerror();
		case 0: // child
			break;
		default: // parent - exit
			throw koalamud::exceptions::forkparent();
	}

	/* Switch process groups to release our parent */
	status = setsid();
	if (status == -1)
	{
		perror("daemonize()::setsid()");
		throw koalamud::exceptions::pgrperror();
	}

	/* close handles */
	fclose(stdin);
	fclose(stdout);
	fclose(stderr);

}

}; /* end koalamud namespace */

/** C++ code entry point.
 * Create our MainServer object and call into it to start the server running.
 */
int main( int argc, char **argv )
{
	try {
		srv = new koalamud::MainServer(argc,argv);

		srv->run();

		delete srv;
	}
	catch (koalamud::exceptions::forkparent &p) {
		cout << "Successfully launched server into background" << endl;
		delete srv;
	}
	catch (koalamud::exceptions::daemonize &d) {
		cerr << "Problem backgrounding server!" << endl;
	}
	catch (...) {
		cerr << "Undefined exception, shutting down" << endl;
		delete srv;
	}

	return 0;
}
