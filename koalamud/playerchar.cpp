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
#include <qsqldatabase.h>

#include <iostream>

#include <unistd.h>

#include "main.hxx"
#include "event.hxx"
#include "playerchar.hxx"
#include "cmdtree.hxx"

/* {{{ Constructor */
K_PlayerChar::K_PlayerChar(int socket, QObject *parent = NULL,
				const char *name = NULL)
  : KoalaDescriptor(socket, parent, name), KRefObj(),
			_state(STATE_GETNAME),
			_disconnecting(false), _indatabase(false), cmdtaskrunning(false)
{
    if (srv->usegui())
    {
			plrstatuslistitem = new QListViewItem(srv->statwin()->PlayerStatusList,
				"Unnamed", "0", "Get Name");
			srv->statwin()->updateplayercount();
    }

	/* Add ourself to the connected player list */
	connectedplayerlist.append(this);

	/* Temporarily fill in our name */
	setName(QString("Unnamed"));

	/* Send welcome message */
	sendWelcome();
}
/* }}} Constructor */

/* {{{ Destructor */
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
    if (srv->usegui())
    {
			srv->statwin()->updateplayercount();
    }
}
/* }}} Destructor */

/* {{{ Update gui status */
void K_PlayerChar::updateguistatus(void)
{
	if (srv->usegui())
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
			case STATE_NEWPLAYER:
				plrstatuslistitem->setText(2, QString("New Player"));
				break;
			case STATE_NEWPLAYERCONFIRM:
				plrstatuslistitem->setText(2, QString("Confirm New Player"));
				break;
			case STATE_NEWPASS:
				plrstatuslistitem->setText(2, QString("Select Password"));
				break;
			case STATE_NEWPASSCONFIRM:
				plrstatuslistitem->setText(2, QString("Confirm Password"));
				break;
			case STATE_GETEMAIL:
				plrstatuslistitem->setText(2, QString("Enter Email"));
				break;
			case STATE_EMAILCONFIRM:
				plrstatuslistitem->setText(2, QString("Confirm Email"));
				break;
			case STATE_GETPASS:
				plrstatuslistitem->setText(2, QString("Get Password"));
				break;
		}
	}
}
/* }}} Update gui status */

/* {{{ readclient */
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
				srv->executor()->execute(new K_PCInputTask(this));
			}
			linequeuelock.release();
		}


		if (_disconnecting)
		{
			close();
			delete this;
		}
}
/* }}} readclient */

/* {{{ setName */
void K_PlayerChar::setName(QString name)
{
	K_Char::setName(name);
}
/* }}} setName */

/* {{{ event */
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
/* }}} event */

/* {{{ sendWelcome */
void K_PlayerChar::sendWelcome(void)
{
	/* FIXME: This should be spruced up later */
	QTextStream os(this);
	os << "Welcome to " << srv->versionstring() << endl;
	os << "By what name are you known? ";
}
/* }}} sendWelcome */

/* {{{  parseline */
void K_PlayerChar::parseline(QString line, QString cline, QString cmdword)
{
	switch (_state)
	{
		case STATE_GETNAME:
		{ /*  */
			if (cmdword.lower() == "new")
			{
				QString toch;
				QTextOStream os(&toch);
				/* We'll need to make this nicer eventually */
				os << "Welcome to the Realm!" << endl;
				os << "Please choose a name for your character: ";
				sendtochar(toch);
				_state = STATE_NEWPLAYER;
			} else {
				/* Technically we should check the database to make sure this name
				 * exists.  We'll get to that later */
				setName(cmdword);
				QString toch;
				QTextOStream os(&toch);
				os << "Please enter your password: ";
				sendtochar(toch);
				_state = STATE_GETPASS;
			}
		} /*  */
		break;
		case STATE_NEWPLAYER:
		{ /*  */
			setName(cmdword);
			QString toch;
			QTextOStream os(&toch);
			os << "You have chosen the name '" << _name << "' for your character.";
			os << endl << "Is this what you want? ";
			sendtochar(toch);
			_state = STATE_NEWPLAYERCONFIRM;
		} /*  */
		break;
		case STATE_NEWPLAYERCONFIRM:
		{
			if (cmdword.lower()[0] == 'y')
			{
				QString toch;
				QTextOStream os(&toch);
				os << "Please select a password for your character." << endl;
				os << "Note:  The administrators have no way to recover your ";
				os << "password if you loose it." << endl;
				os << "Password: ";
				sendtochar(toch);
				_state = STATE_NEWPASS;
			} else {
				QString toch;
				QTextOStream os(&toch);
				os << "By what name are you known? ";
				sendtochar(toch);
				_state = STATE_GETNAME;
			}
		}
		break;
		case STATE_NEWPASS:
		{
			_password = cline;
			QString toch;
			QTextOStream os(&toch);
			os << "Confirm password: ";
			sendtochar(toch);
			_state = STATE_NEWPASSCONFIRM;
		} 
		break;
		case STATE_NEWPASSCONFIRM:
		{ /*  Confirm Password */
			if (_password != cline)
			{
				QString toch;
				QTextOStream os(&toch);
				os << "We're sorry, but those passwords didn't match,";
				os << "Please try again." << endl;
				sendtochar(toch);
				_state = STATE_NEWPASS;
			} else {
				/* Passwords match, request email address */
				QString toch;
				QTextOStream os(&toch);
				os <<
"Your email address is used only to forward system status updates" << endl <<
"and to communicate with you in the event that you need your" << endl <<
"password reset." << endl << 
"We will never sell your email address or use it to send any" << endl <<
"Unsolicited emails." << endl <<
"Please enter your correct email address: ";
				sendtochar(toch);
				_state = STATE_GETEMAIL;
			}
		} /*  */
		break;
		case STATE_GETEMAIL:
		{
			_email  = cline;
			QString toch;
			QTextOStream os(&toch);
			os << "You entered '" << _email << "' as your email address." << endl;
			os << "Is this correct? ";
			sendtochar(toch);
			_state = STATE_EMAILCONFIRM;
		}
		break;
		case STATE_EMAILCONFIRM:
		{
			if (cmdword.lower()[0] == 'y')
			{
				QString toch;
				QTextOStream os(&toch);
				os <<
"Thank you for joining KoalaMud.  We hope your stay will be an" << endl <<
"enjoyable one.  If you have any questions, don't hessitate to ask." << endl;
				sendtochar(toch);
				save();
				_indatabase = true;
				_state = STATE_PLAYING;
				/* Playermap is updated when the player goes into STATE_PLAYING */
				connectedplayermap.insert(_name, this);
				/* Join gossip channel if it exists */
				if (channelmap["gossip"] != NULL)
				{
					channelmap["gossip"]->joinchannel(this);
				}
			} else {
				QString toch;
				QTextOStream os(&toch);
				os <<
"Your email address is used only to forward system status updates" << endl <<
"and to communicate with you in the event that you need your" << endl <<
"password reset." << endl << 
"We will never sell your email address or use it to send any" << endl <<
"Unsolicited emails." << endl <<
"Please enter your correct email address: ";
				sendtochar(toch);
				_state = STATE_GETEMAIL;
			}
		}
		break;
		case STATE_GETPASS:
			_password = cline;
			if (!load())
			{
				/* Bad password */
				QString toch;
				QTextOStream os(&toch);
				os << "Invalid Password!" << endl;
				os << "Please enter the name of your character, or 'new' to start ";
				os << "a new character." << endl;
				os << "By what name are you known? ";
				sendtochar(toch);
				_state = STATE_GETNAME;
				break;
			}
			_indatabase = true;
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
			/* We use the command tree to search for our command, then call the
			 * returned pointer's run function */
			koalamud::Command *cmd = maincmdtree->findandcreate(cmdword, this, true);
			if (cmd)
			{
				/* FIXME:  We will need to pass the remainder of our input into this
				 * function */
				cmd->run(cmdword, cline.section(' ', 1));
				delete cmd;
			} else {
				/* Output an error message */
				QTextStream os(this);
				os << _name << ", that command is unknown." << endl;
			}
			break;
	}
}
/* }}} parseline */

/* {{{ channelsendtochar */
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
		if (from != NULL)
		{
			outmsg = templateall.replace(QRegExp("%sender%"),
						from->getName(this) );
		}
		outmsg = outmsg.replace(QRegExp("%message%"), msg);
		QString endline;
		QTextOStream os(&endline);
		os << endl;
		sendtochar(outmsg);
		sendtochar(endline);
	}
}
/* }}} channelsendtochar */

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

/* {{{ save */
void K_PlayerChar::save(void)
{
	QSqlQuery savequery;

	if (!_indatabase)
	{ /* {{{ */
		/* This is the first time the player object is being saved, insert
		 * everything into the database */
		QString q;
		QTextOStream qos(&q);
		qos << "insert into players (name, pass, email, created, lastlogin)";
		qos << " values" << endl;
		qos << "('" << _name << "', MD5('" << _password << "')," << endl;
		qos << "'" << _email << "', NOW(), NOW());";
		if (!savequery.exec(q))
		{
			cout << "Error saving " << _name << " to the database!" << endl;
			cout << "Query: " << q << endl;
		sendtochar("There was a problem recording your deeds, please try again.");
		}
		_indatabase = true;
		/* }}} */
	} else {
		/* {{{ */
		QString q;
		QStringList setlist;
		QTextOStream qos(&q);
		qos << "update players" << endl;
		if (_password.length() > 4)
		{
			QString si;
			QTextOStream sios(&si);
			sios << "set pass = MD5('" << _password << "')";
			setlist += si;
		}
		qos << setlist.join(", ");
		qos << "where name = '" << _name << "';";
		if (!savequery.exec(q))
		{
			cout << "Error saving " << _name << " to the database!" << endl;
			cout << "Query: " << q << endl;
		sendtochar("There was a problem recording your deeds, please try again.");
		}
		/* }}} */
	}
}
/* }}} save */

/* {{{ Load */
/* This should only be called at login */
bool K_PlayerChar::load(bool checkpass = true)
{
	/* At this point, we should have the player name and password in the class.
	 * We'll do a select limiting on both to get the record.  If the query
	 * fails, we return false to signify an invalid login.  checkpass provides a
	 * means for bypassing the password check if an imm needs to force a reload
	 * from the database for some reason */
	QString q;
	QTextOStream qos(&q);

	/* Note, we individually list fields so that we know the proper order */
	qos << "select name, email" << endl;
	qos << "from players" << endl;
	qos << "where name = '" << _name << "'" << endl;
	if (checkpass)
	{
		qos << "and pass = MD5('" << _password << "')";
	}
	qos << ";";

	QSqlQuery loadq(q);
	if (loadq.isActive())
	{
		if (loadq.numRowsAffected() == 0)
		{
			return false;
		}
		/* There will only be one record, the index on name ensures that */
		loadq.next();
		_email = loadq.value(1).toString();
		_name = loadq.value(0).toString(); // So we get the case correct
		return true;  // let the caller update lastlogin if needed
	} else {
		return false;
	}
}
/* }}} Load */

/* {{{ Input task implementation */
K_PCInputTask::K_PCInputTask(K_PlayerChar *plrchar)
		: ch(plrchar)
{
}

K_PCInputTask::K_PCInputTask(KRefPtr<K_PlayerChar> &plrchar)
		: ch(plrchar)
{
}

K_PCInputTask::~K_PCInputTask()
{
}

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
	QThread::postEvent(*ch, new QCustomEvent(EVENT_CHAR_OUTPUT));

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
		srv->executor()->execute(new K_PCInputTask(ch));
	}
	ch->linequeuelock.release();
}
/* }}} Input task implementation */
