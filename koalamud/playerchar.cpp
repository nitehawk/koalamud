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

#include <qregexp.h>

#include <iostream>

#include <unistd.h>

#include "event.hxx"
#include "playerchar.hxx"

K_PlayerChar::K_PlayerChar(int socket, QObject *parent = NULL,
				const char *name = NULL)
  : KoalaDescriptor(socket, parent, name), _state(STATE_GETNAME),
			_disconnecting(false), cmdtaskrunning(false)
{
    if ( guiactive )
    {
			plrstatuslistitem = new QListViewItem(stat->PlayerStatusList,
				"Unnamed", "0", "Get Name");
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

		/* Remove ourself from the player lists */
		connectedplayerlist.removeRef(this);
		connectedplayermap.remove(_name);

		/* Cleanup our status window information */
    delete plrstatuslistitem;
    if (guiactive)
    {
			stat->updateplayercount();
    }
}

void K_PlayerChar::updateguistatus(void)
{
	if (guiactive)
	{
		plrstatuslistitem->setText(0, _name);
		switch (_state)
		{
			case STATE_GETNAME:
				plrstatuslistitem->setText(2, QString("Get Name"));
				break;
			case STATE_PLAYING:
				plrstatuslistitem->setText(2, QString("Playing"));
				break;
		}
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
			
			/* Create and populate the textin object */
			plrtextin_pt ti = new plrtextin_t;
			ti->line = line;
			ti->cline = cline;
			ti->cmdword = word;

			/* push it onto the queue and schedule the task - Scheduling inside the
			 * lock allows us to use the lock for both securing the queue and
			 * protecting cmdtaskrunning */
			linequeuelock.acquire();
			lineinqueue.enqueue(ti);
			if (cmdtaskrunning == false)
			{
				cmdtaskrunning = true;
				executor->execute(new K_PCInputTask(this));
			}
			linequeuelock.release();
		}


		if (_disconnecting)
		{
			close();
			delete this;
		}
}

bool K_PlayerChar::event(QEvent *e)
{
	static bool disconeventposted = false;
	switch (e->type())
	{
		case EVENT_CHAR_OUTPUT:
			if (_disconnecting)
			{
				if (disconeventposted)
					delete this;
				else
				{
					disconeventposted = true;
					QThread::postEvent(this, new QCustomEvent(EVENT_CHAR_OUTPUT));
				}
			} else {
				updateguistatus();
			}
			return true;
		default:
			return false;
	}
}

void K_PlayerChar::setName(QString name)
{
	/* Fall through to parent implementation */
	K_Char::setName(name);
}

void K_PlayerChar::runcmd(cmdentry_t *cmd, QString word, QString args)
{
	if (cmd == NULL)
		return;

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

void K_PlayerChar::parseline(QString line, QString cline, QString cmdword)
{
	switch (_state)
	{
		case STATE_GETNAME:
			setName(cmdword);
			_state = STATE_PLAYING;
			/* Playermap is updated when the player goes into STATE_PLAYING */
			connectedplayermap.insert(_name, this);
			/* Join gossip channel if it exists */
			if (channelmap["gossip"] != NULL)
			{
				channelmap["gossip"]->joinchannel(this);
			}
			break;
		case STATE_PLAYING:
			/* If the command exists, run it using 'this->runcmd()' */
			cmdentry_t *cmd = cmddict[cmdword];
			if (cmd)
			{
				runcmd(cmd, cmdword, cline.section(' ', 1));
			} else
			{
				/* Output an error message */
				QTextStream os(this);
				os << _name << ", that command is unknown." << endl;
			}
			break;
	}
}

/* Replace template elements with appropriate strings and send on to char */
void K_PlayerChar::channelsendtochar(K_PlayerChar *from, QString templateall,
				QString templatesender, QString msg)
{
	if (from == this)
	{
		/* This is the version to ourself, fill in template sender and send it on
		 */
		QString outmsg;
		outmsg = templatesender.replace(QRegExp("%sender%"), "You");
		outmsg = outmsg.replace(QRegExp("%message%"), msg);
		QString endline;
		QTextOStream os(&endline);
		os << endl;
		sendtochar(outmsg);
		sendtochar(endline);
		return;
	} else
	{
		QString outmsg;
		outmsg = templateall.replace(QRegExp("%sender%"),
					from->getName(this) );
		outmsg = outmsg.replace(QRegExp("%message%"), msg);
		QString endline;
		QTextOStream os(&endline);
		os << endl;
		sendtochar(outmsg);
		sendtochar(endline);
	}
}

/* When we start tracking subscribed channels, this will remove chan from the
 * list */
void K_PlayerChar::channeldeleted(KoalaChannel *chan)
{

}

/* Use this to send to send all strings to the character.  This isolates
 * colorize operations to a single function when we start handling color */
bool K_PlayerChar::sendtochar(QString text)
{
	QTextStream os(this);
	os << text;
}

/* {{{ Input task implementation */
/* Run an input task and queue another event if more lines of input are
 * available */
void K_PCInputTask::run(void) throw()
{
	/* Two checks for this, one at the beginning, and one at the end - This
	 * makes sure that 'quit' doesn't end up getting ignored */
	if (ch->_disconnecting)
	{
		//delete ch;
		return;
	}

	/* Get a command from the list */
	ch->linequeuelock.acquire();
	plrtextin_pt ti = ch->lineinqueue.dequeue();
	ch->linequeuelock.release();

	/* Hand back over to the PC object for processing */
	ch->parseline(ti->line, ti->cline, ti->cmdword);
	
	/* cleanup the task object */
	delete ti;

	/* Make sure output gets processed properly */
	QThread::postEvent(ch, new QCustomEvent(EVENT_CHAR_OUTPUT));

	/* Two checks for this, one at the beginning, and one at the end - This
	 * makes sure that 'quit' doesn't end up getting ignored */
	if (ch->_disconnecting)
	{
		//delete ch;
		return;
	}

	/* Check for another line of input and reset the cmdtaskrunning flag if
	 * there aren't any more */
	ch->linequeuelock.acquire();
	if (ch->lineinqueue.isEmpty())
	{
		ch->cmdtaskrunning = false;
	} else {
		/* Schedule another task */
		executor->execute(new K_PCInputTask(ch));
	}
	ch->linequeuelock.release();
}
/* }}} Input task implementation */
