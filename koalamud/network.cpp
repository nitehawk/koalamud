
/***************************************************************\
*                        KoalaMud Gen 2                         *
*    Copyright (c) 2002 First Step Internet Services, Inc.      *
*                     All Rights Reserved                       *
*        Distributed under the terms of the FSI License         *
* See file LICENSE in the root of this package for information  *
\***************************************************************/
/***************************************************************\
*	Module:
* Description:
* Classes:
\***************************************************************/

#define KOALA_NETWORK_CXX "%A%"

#include "main.hxx"
#include "network.hxx"
#include "playerchar.hxx"

KoalaServer::KoalaServer(unsigned int port, porttype_t newporttype = GAMESERVER)
: QServerSocket(port), _type(newporttype), _port(port)
{
    /* Create a list item if the gui is active */
    if (guiactive)
    {
	QString portstr;
	portstr.setNum(port);
	ListenStatusItem = new QListViewItem(stat->ListenStatusList,	portstr, "Online");
	stat->updatelistencount();
    }
}

KoalaServer::~KoalaServer()
{
	/* delete the list item for the port */
	delete ListenStatusItem;
	if (guiactive)
	{
	    stat->updatelistencount();
	}
	/* remove ourself from the list of listeners */
}

/** Handle a newly accepted connection and bring it into the game world */
void KoalaServer::newConnection(int socket)
{
  new K_PlayerChar(socket);
}


/* Construct a new descriptor */
KoalaDescriptor::KoalaDescriptor(int sock, QObject * parent=NULL, const char *name=NULL)
      : QSocket( parent, name )
{
  connect(this, SIGNAL(readyRead()), SLOT(readclient()));
  connect(this, SIGNAL(connectionClosed()), SLOT(connectionclosed()));
  setSocket(sock);
}

KoalaDescriptor::~KoalaDescriptor(void)
{
}

void KoalaDescriptor::readclient(void)
{
  /* Base class definition will just act as an echo port */
  while ( canReadLine() )
  {
    QTextStream os( this );
    os << "Read: " << readLine();
  }
}

void KoalaDescriptor::connectionclosed(void)
{
  delete this;
}
