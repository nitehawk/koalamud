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
#include "comm.hxx"

KOALACMD(cmd_quit)
{
	QString str;
	QTextOStream os(&str);

	os << "Thank you for playing on KoalaMud." << endl;
	os << "Please return soon." << endl;
	ch->sendtochar(str);

	ch->setdisconnect(true);
}

KOALACMD(cmd_who)
{
	QString str;
	QTextOStream os(&str);

	/* Grab an iterator to the player list and loop through it */
	playerlistiterator_t pli(connectedplayerlist);
	K_PlayerChar *cur;
	os << "The following players are online: " << endl;
	while ((cur = pli.current()) != NULL)
	{
		++pli;
		/* Later we'll want to display more information as well as check
		 * imm invisibility and a lot of other stuff before displaying someone */
		os << cur->getName(ch) << endl;
	}

	ch->sendtochar(str);
}

void initcmddict(void)
{
	cmddict.insert(QString("quit"), new cmdentry_t("quit", cmd_quit));
	cmddict.insert(QString("who"), new cmdentry_t("who", cmd_who));
	initcommcmddict();
}
