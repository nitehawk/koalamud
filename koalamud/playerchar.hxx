/***************************************************************\
*                        KoalaMud Gen 2                         *
*    Copyright (c) 2002 First Step Internet Services, Inc.      *
*                     All Rights Reserved                       *
*        Distributed under the terms of the FSI License         *
* See file LICENSE in the root of this package for information  *
\***************************************************************/
/***************************************************************\
*	Module:
*	Author:
* Description:
* Classes:
\***************************************************************/

#ifndef KOALA_PLAYERCHAR_HXX
#define KOALA_PLAYERCHAR_HXX "%A%"

#include <qobject.h>
#include <qlistview.h>
#include <qptrlist.h>

#include "main.hxx"
#include "char.hxx"
#include "network.hxx"
#include "koalastatus.h"

class K_PlayerChar;
#include "cmd.hxx"

/**
 * Player Character object - Handles interaction that is specific to player characters.
 * Matthew Schlegel
 **/
class K_PlayerChar : public virtual KoalaDescriptor, public virtual K_Char
{
    Q_OBJECT

		public:
		typedef enum {STATE_GETNAME, STATE_PLAYING} state_t;

		protected:
		state_t _state;
    
    public:
    K_PlayerChar(int sock, QObject *parent=0, const char *name=0);
    virtual ~K_PlayerChar();
		virtual void setName(QString name);
		virtual void sendWelcome(void);
    
    private slots:
    virtual void readclient(void);

		public slots:
		virtual void setdisconnect(bool dis = true) { _disconnecting = dis; }
		virtual void runcmd(cmdentry_t *cmd, QString word, QString arg);
    
    protected:
    QListViewItem *plrstatuslistitem;
		bool _disconnecting;
};

typedef QPtrList<K_PlayerChar> playerlist_t;
typedef QPtrListIterator<K_PlayerChar> playerlistiterator_t;

#ifdef KOALA_PLAYERCHAR_CXX
playerlist_t connectedplayerlist;
#else
extern playerlist_t connectedplayerlist;
#endif

#endif  //  KOALA_PLAYERCHAR_HXX
