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

#define KOALA_CMD_CXX "%A%"

#include "cmd.hxx"

/* command prototypes - FIXME: Temp */
KOALACMD(cmd_quit);
KOALACMD(cmd_who);

void initcmddict(void)
{
	cmddict.insert(QString("quit"), new cmdentry_t("quit", cmd_quit));
	cmddict.insert(QString("who"), new cmdentry_t("who", cmd_who));
}

KOALACMD(cmd_quit)
{
	ch->setdisconnect(true);
}

KOALACMD(cmd_who)
{
	/* Grab an iterator to the player list and loop through it */
	playerlistiterator_t pli(connectedplayerlist);
	K_PlayerChar *cur;
	QTextStream os(ch);
	os << "The following players are online: " << endl;
	while ((cur = pli.current()) != NULL)
	{
		++pli;
		os << cur->getName() << endl;
	}
}
