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

K_PlayerChar::K_PlayerChar(int socket, QObject *parent = NULL,
				const char *name = NULL)
  : KoalaDescriptor(socket, parent, name), _state(STATE_GETNAME),
			_disconnecting(false)
{
    if ( guiactive )
    {
	plrstatuslistitem = new QListViewItem(stat->PlayerStatusList, "Unnamed", "0");
	stat->updateplayercount();
    }

	/* Add ourself to the connected player list */
	connectedplayerlist.append(this);

	/* Temporarily fill in our name */
	setName(QString("Unnamed"));

	/* Send welcome message */
	sendWelcome();
}

K_PlayerChar::~K_PlayerChar()
{
		/* At some point we will probably want to handle linkless players.  For
		 * now loosing link means the player is logged off as normal which will
		 * autosave (later) */

		/* Remove ourself from the player list */
		connectedplayerlist.removeRef(this);

		/* Cleanup our status window information */
    delete plrstatuslistitem;
    if (guiactive)
    {
			stat->updateplayercount();
    }
}

void K_PlayerChar::readclient(void)
{
    while ( canReadLine() && (_disconnecting == false))
    {
			/* line is used for some states and commands, word and cline are used
			 * otherwise */
			QString line = readLine();
			QString cline = line.simplifyWhiteSpace();
			QString word = cline.section(' ', 0, 0);

			switch (_state)
			{
				case STATE_GETNAME:
					setName(word);
					_state = STATE_PLAYING;
					break;
				case STATE_PLAYING:
					/* If the command exists, run it using 'this->runcmd()' */
					cmdentry_t *cmd = cmddict[word];
					if (cmd)
					{
						runcmd(cmd, word, cline.section(' ', 1));
					} else
					{
						/* Output an error message */
						QTextStream os(this);
						os << _name << ", that command is unknown." << endl;
					}
					break;
			}
    }

		if (_disconnecting)
		{
			close();
			delete this;
		}
}

void K_PlayerChar::setName(QString name)
{
	/* Fall through to parent implementation */
	K_Char::setName(name);
	/* Handle gui tasks */
	if (guiactive)
	{
		/* Change our name in the status list */
		plrstatuslistitem->setText(0, _name);
	}
}

void K_PlayerChar::runcmd(cmdentry_t *cmd, QString word, QString args)
{
	/* FIXME: */
	/* We should check that everything is ok for running the command here */

	cmd->cmdfunc(this, word, args);
}

void K_PlayerChar::sendWelcome(void)
{
	/* FIXME: This should be spruced up later */
	QTextStream os(this);
	os << "Welcome to KoalaMud Gen 2 v0.3.0a" << endl;
	os << "By what name are you known? ";
}
