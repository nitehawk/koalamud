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

#include "main.hxx"
#include "char.hxx"
#include "network.hxx"
#include "koalastatus.h"

/**
 * Player Character object - Handles interaction that is specific to player characters.
 * Matthew Schlegel
 **/
class K_PlayerChar : public virtual KoalaDescriptor, public virtual K_Char
{
    Q_OBJECT
    
    public:
    K_PlayerChar(int sock, QObject *parent=0, const char *name=0);
    virtual ~K_PlayerChar();
    
    private slots:
    virtual void readclient(void);
    
    protected:
    QListViewItem *plrstatuslistitem;
};

#endif  //  KOALA_PLAYERCHAR_HXX