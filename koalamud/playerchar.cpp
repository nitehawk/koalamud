/***************************************************************\
*                        KoalaMud Gen 2                         *
*    Copyright (c) 2002 First Step Internet Services, Inc.      *
*                     All Rights Reserved                       *
*        Distributed under the terms of the FSI License         *
* See file LICENSE in the root of this package for information  *
\***************************************************************/
/***************************************************************\
* Module:
* Author:
* Description:
* Classes:
\***************************************************************/

#define KOALA_PLAYERCHAR_CXX "%A%"

#include <iostream>

#include "playerchar.hxx"

K_PlayerChar::K_PlayerChar(int socket, QObject *parent = NULL, const char *name = NULL)
  : KoalaDescriptor(socket, parent, name)
{
    if ( guiactive )
    {
	plrstatuslistitem = new QListViewItem(stat->PlayerStatusList, "Unnamed", "0");
	stat->updateplayercount();
    }
}

K_PlayerChar::~K_PlayerChar()
{
    delete plrstatuslistitem;
    if (guiactive)
    {
	stat->updateplayercount();
    }
}

void K_PlayerChar::readclient(void)
{
    while ( canReadLine() )
    {
	QTextStream os( this );
	os << "Read: " << readLine();
    }
}
