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
#include "network.hxx"
#include "koalastatus.h"

int main( int argc, char **argv )
{
  guiactive = true;

    /* FIXME:  we need to scan argv for an argument to turn off the gui */

  QApplication a( argc, argv, guiactive );

   if (guiactive) {
     stat = new KoalaStatus();
     stat->statusBar()->message("online");
     stat->show();
     a.setMainWidget(stat);
   }

   return a.exec();
}
