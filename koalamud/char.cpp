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

#define KOALA_CHAR_CXX "%A%"

#include <qregexp.h>

#include "main.hxx"
#include "char.hxx"
#include "event.hxx"

namespace koalamud
{

/** Get the characters name.
 * If @a pair is specified, this will check if the character is visible to the
 * character pointed to by @a pair.  If they are visible, it will next check
 * to see if they know each other.
 * @note getVoice (currently unimplemented) will return a string for the voice
 * of a character.   getVoice should be used in all comm functions.  getName
 * should be used anywhere where you would 'see' the character (listed in
 * room, looking at character, etc.)
 */
QString Char::getName(Char *pair=NULL)
{
	if (pair == NULL)
		return _name;

	/* Check visibility - Simple for now */
	if (visibleTo(pair))
	{
		return _name;
	} else {
		return "someone";
	}
}

/** Is this character visible to the @a pair character. */
bool Char::visibleTo(Char *pair)
{
	return true;
}

/** Attach a descriptor to character */
void Char::setDesc(ParseDescriptor *desc)
{
	/* Disconnect all signals from this descriptor */
	if (_desc)
	{
		disconnect(_desc);
	}
	_desc = desc;
	/* Connect close signals */
	connect(desc, SIGNAL(destroyed()), this, SLOT(descriptorClosed()));
}

/** Send a message from a channel on to the char */
void Char::channelsendtochar(Char *from, QString templateall,
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
		sendtochar(endline);
		sendtochar(outmsg);
		sendtochar(endline);
		return;
	} else
	{
		QString outmsg;
		if (from != NULL)
		{
			outmsg = templateall.replace(QRegExp("%sender%"),
						from->getName(this) );
		}
		outmsg = outmsg.replace(QRegExp("%message%"), msg);
		QString endline;
		QTextOStream os(&endline);
		os << endl;
		sendtochar(endline);
		sendtochar(outmsg);
		sendtochar(endline);
		sendPrompt();
	}
}

/** Add a command to the command queue
 * Also make sure a task is in existance to eventually run it.
 */
void Char::queueCommand(Command *cmd, QString args)
{
	cmdqueueitem *newitem = new cmdqueueitem;
	newitem->cmd = cmd;
	newitem->args = args;

	cmdqueuelock.acquire();
	cmdqueue.enqueue(newitem);
	if (!cmdtaskrunning)
	{
		cmdtaskrunning = true;
		/* Queue task */
		srv->executor()->execute(new cmdexectask(this));
	}
	cmdqueuelock.release();
}

bool Char::sendtochar(QString data)
{
	if (_desc)
	{
		_desc->send(data);
		_desc->notifyOutput(this);
		return true;
	}
	return false;
}

/* {{{ cmdexectask implementation */
/** Initialize a command execution task */
void Char::cmdexectask::run(void)
{
	/* Don't do anything if we are are a player and disconnecting */
	if (_ch->isDisconnecting() && _ch->isPC())
		return;

	/* Get the first pending command */
	_ch->cmdqueuelock.acquire();
	cmdqueueitem *cmditem = _ch->cmdqueue.dequeue();
	_ch->cmdqueuelock.release();

	/* run command */
	try {
		cmditem->cmd->runCmd(cmditem->args);
	}
	catch (koalamud::exceptions::cmdpermdenied p)
	{
		QString out;
		QTextOStream os(&out);
		os << "You do not have permission to run this command." << endl;
		_ch->sendtochar(out);
	}
	_ch->sendPrompt();

	/* cleanup */
	delete cmditem->cmd;
	delete cmditem;
	
	/* Don't schedule any more command executors if we are disconnecting */
	if (_ch->isDisconnecting())
		return;

	/* If there are more commands pending, start another executor */
	_ch->cmdqueuelock.acquire();
	if (_ch->cmdqueue.isEmpty())
	{
		_ch->cmdtaskrunning = false;
	} else {
		srv->executor()->execute(new cmdexectask(_ch));
	}
	_ch->cmdqueuelock.release();
}

/* }}} end cmdexectask implementation */

}; /* End koalamud namespace */
