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

#include "main.hxx"
#include "database.hxx"
#include "network.hxx"
#include "koalastatus.h"
#include "cmd.hxx"
#include "memory.hxx"

int main( int argc, char **argv )
{
	/* Seed the allocator with some pool sizes to get things going */
	poolalloc.createpool(260);

	cout << poolalloc << endl;

	executor = new ZThread::PoolExecutor<false>(TPMIN, TPMAX);
  guiactive = true;

  /* FIXME:  we need to scan argv for an argument to turn off the gui */
  QApplication a( argc, argv, guiactive );

	/* Start database */
	if (!initdatabasesystem())
	{
		/* Houston, we have a problem! */
		cerr << "FATAL:  Unable to connect to SQL server!" << endl;
		executor->cancel();
		executor->join();
		return 1;
	}

   if (guiactive) {
     stat = new KoalaStatus();
     stat->statusBar()->message("online");
     stat->show();
     a.setMainWidget(stat);
   }

	/* Load commands */
	initcmddict();
	

	/* Start a listener */
	new KoalaServer(4444);

  int res = a.exec();
	executor->cancel();
	executor->join();
	cout << poolalloc << endl;
	return res;
}
