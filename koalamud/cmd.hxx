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

#ifndef KOALA_CMD_HXX
#define KOALA_CMD_HXX "%A%"

#include <qdict.h>
#include <qstring.h>

class cmdentry_t;
#include "playerchar.hxx"

typedef void (*cmdfunc_t)(K_PlayerChar *, QString, QString);

#define KOALACMD(cmdn) void cmdn(K_PlayerChar *ch, QString cmd, QString args)

class cmdentry_t {
	public:
	QString command;
	cmdfunc_t cmdfunc;
	cmdentry_t(QString cmd, cmdfunc_t ptr) : command(cmd), cmdfunc(ptr) {}
};

#ifdef KOALA_CMD_CXX
QDict<cmdentry_t> cmddict(2531, FALSE);
#else
extern QDict<cmdentry_t> cmddict;
#endif

void initcmddict(void);

#endif // KOALA_CMD_HXX
