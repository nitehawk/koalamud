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

class KoalaServer : public QServerSocket
{
	public:
		typedef enum { GAMESERVER } porttype_t;

public: // Public methods
   KoalaServer(unsigned int port, porttype_t = GAMESERVER);
   ~KoalaServer();

  /** Handle a newly accepted connection and bring it into the game world */
  virtual void newConnection(int socket);

protected:
	QListViewItem *ListenStatusItem;
	porttype_t _type;
	unsigned int _port;
};

class KoalaDescriptor : public QSocket
{
    Q_OBJECT
  
  public:
  KoalaDescriptor(int sock, QObject *parent=0, const char *name=0);
  virtual ~KoalaDescriptor();
  
  private slots:
	virtual void readclient(void);
  virtual void connectionclosed(void);
};

#endif  // KOALA_NETWORK_HXX
