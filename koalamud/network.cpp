
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
#include "logging.hxx"
#include "network.hxx"
#include "playerchar.hxx"

/** Initialize a listener object
 * Setup a port listener and update the gui status
 */
KoalaServer::KoalaServer(unsigned int port, porttype_t newporttype = GAMESERVER)
: QServerSocket(port), _type(newporttype), _port(port)
{
    /* Create a list item if the gui is active */
    if (srv->usegui())
    {
			QString portstr;
			portstr.setNum(port);
			ListenStatusItem = new QListViewItem(srv->statwin()->ListenStatusList,
									portstr, "Online");
			srv->statwin()->updatelistencount();
    }
}

/** Destroy a listener object */
KoalaServer::~KoalaServer()
{
	/* delete the list item for the port */
	delete ListenStatusItem;
	if (srv->usegui())
	{
	    srv->statwin()->updatelistencount();
	}
	/* remove ourself from the list of listeners */
}

/** Handle a newly accepted connection and bring it into the game world */
void KoalaServer::newConnection(int socket)
{
	Logger::msg("Accepting new Player connection.");
	/* FIXME:  This should probably go through a factory of some variety to
	 * provide some flexibility */
  new K_PlayerChar(socket);
}


/** Construct a new descriptor */
KoalaDescriptor::KoalaDescriptor(int sock, QObject * parent=NULL, const char *name=NULL)
      : QSocket( parent, name )
{
  connect(this, SIGNAL(readyRead()), SLOT(readclient()));
  connect(this, SIGNAL(connectionClosed()), SLOT(connectionclosed()));
  setSocket(sock);
}

/** Nothing to do in the destructor */
KoalaDescriptor::~KoalaDescriptor(void)
{
}

/** Read data from the socket and echo it back to the socket
 * @todo Once this code is migrated into the new design, this will need to be
 * changed into a more complete implementation.
 */
void KoalaDescriptor::readclient(void)
{
  /* Base class definition will just act as an echo port */
  while ( canReadLine() )
  {
    QTextStream os( this );
    os << "Read: " << readLine();
  }
}

/** Respond to a closed connection
 * Currently we just destroy any descriptors that are closed.  In the future
 * there will be a bit more cleanup and notifications to be sent.
 */
void KoalaDescriptor::connectionclosed(void)
{
  delete this;
}
