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
#include "exception.hxx"
#include "database.hxx"
#include "network.hxx"
#include "koalastatus.h"
#include "cmdtree.hxx"

namespace koalamud {

/** Main server constructor
 * Here we initialize and prepare the various subsystems and load all of the
 * server configuration information from the database.
 */
MainServer::MainServer( int argc, char **argv ) throw(koalaexception)
	: _executor(NULL), _guiactive(true), _background(false),
		_profile("default")
{
	/* Initialize our task pool executor */
	_executor = new ZThread::PoolExecutor<ZThread::FastMutex>(threadpoolmin, threadpoolmax);

	/* Call to process arguments here */
	if (!parseargs(argc, argv))
		return;

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

}

/** Start everything running */
void MainServer::run(void)
{
	/* This will be replaced with exception handling */
	if (!_kmdb || !_kmdb->isonline())
		return;

	/* Start a listener */
	/* FIXME:  This information should be retrieved from the database */
	new KoalaServer(4444);
	new KoalaServer(6464);

	/* Update status bar */
	if (_guiactive) {
     _statwin->statusBar()->message("online");
   }

	_app->exec();
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
}

/** Return current version string */
QString MainServer::versionstring(void)
{
	return "KoalaMud Gen2 v0.3.5a";
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
"  -g         disable GUI" << endl <<
"  -G         enable GUI (default)" << endl;

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
