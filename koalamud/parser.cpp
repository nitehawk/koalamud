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

#define KOALA_PARSER_CXX "%A%"

#include <qsqlquery.h>

#include "parser.hxx"
#include "main.hxx"
#include "network.hxx"
#include "playerchar.hxx"
#include "cmdtree.hxx"

namespace koalamud
{

/** Build a player login parser
 * We also send our welcome message from here.
 */
PlayerLoginParser::PlayerLoginParser(Char *ch=NULL, ParseDescriptor *desc=NULL)
	: Parser(ch, desc), state(STATE_GETNAME)
{
	/* If desc is null, we really have other problems anyway.  Nothing that
	 * would explicitly cause failure though. */
	if (_desc)
	{
		QString query;
		QTextOStream qos(&query);
		QSqlQuery q;
		qos << "select art from welcomeart order by RAND() limit 1;";
		if (q.exec(query) && q.numRowsAffected() > 0)
		{
			/* Got a welcome screen from the database, send it on to the new
			 * descriptor */
			q.next();
			_desc->send(q.value(0).toString());
		} else {
			/* Query for welcome art failed or no art available.  Send a default
			 * string */
			QString str;
			QTextOStream os(&str);

			os << "Welcome to Shadow of the Wheel!" << endl
				 << "Server running " << srv->versionstring() << endl
				 << "By what name are you known? ";
			_desc->send(str);
		}
	}
}

/** Player login
 * Parse a line of input.  First line is player name - Check to see if it
 * exists, if it does, get password and attempt to spawn a playerchar object.
 * if password is wrong, go back to beginning state, if it is correct, switch
 * parser to 'Playing' parser.
 * @note We confirm that the player wants a new character in this class and
 * pass the name into the next parser.
 */
void PlayerLoginParser::parseLine(QString line)
{
	/* Simplify the input line to ensure we don't end up with the \n */
	QString sline = line.simplifyWhiteSpace();
	/* Our output vars */
	QString out;
	QTextOStream os(&out);
	/* Query stuff */
	QSqlQuery q;
	QString query;
	QTextOStream qos(&query);

	/* Handle current imput state */
	switch (state)
	{
		case STATE_GETNAME:
			if (QString("new") == sline.lower()
					|| QString("create") == sline.lower())
			{
				os << endl << "Are you sure you want to create a new character?";
				state = STATE_CONFNEW;
			} else {
				pname = sline;
				/* Confirm that player name exists */
				qos << "Select playerid from players" << endl
					  << "where name = '" << sline << "';";
				if (q.exec(query))
				{
					if (q.numRowsAffected() == 1)
					{
						state = STATE_GETPASS;
						/* FIXME: We should turn off echo here */
						os << endl << "Enter your password: ";
					} else {
						state = STATE_CONFNAME;
						os << endl << "That player does not exist, would you like to "
							 << "create a new character?";
					}
				}
			}
			break;
		case STATE_GETPASS:
			/* Confirm password, if valid, check for existing char object for the
			 * player.  If there is an existing char object, attach to it and cancel
			 * the linkless timer, otherwise create a new char object with the
			 * player name.  With a valid password, we also switch over to the next
			 * parser class.
			 */
			qos << "select playerid from players" << endl
					<< "where name = '" << pname << "' and" << endl
					<< "pass = MD5('" << sline << "');";
			if (q.exec(query))
			{
				if (q.numRowsAffected() == 1)
				{
					_ch = new PlayerChar(pname, _desc);
					_desc->setParser(new PlayerParser(_ch, _desc));
				} else {
					os << endl << "I'm sorry, that password is incorrect." << endl
						 << "By what name are you known? ";
					state = STATE_GETNAME;
				}
			}
			break;
		case STATE_CONFNAME:
			if (QString("yes").startsWith(sline.lower()))
			{
				_desc->setParser(new PlayerCreationParser(_desc, pname));
			} else {
				state = STATE_GETNAME;
				os << endl << "Ok then, What is your name?";
			}
			break;
		case STATE_CONFNEW:
			if (QString("yes").startsWith(sline.lower()))
			{
				_desc->setParser(new PlayerCreationParser(_desc));
			} else {
				state = STATE_GETNAME;
				os << endl << "Ok then, What is your name?";
			}
			break;
	}

	if (!out.isEmpty())
	{
		_desc->send(out);
	}
}

/** Build a player parser
 * This also sends the characters initial prompt
 */
PlayerParser::PlayerParser(Char *ch, ParseDescriptor *desc)
	: Parser(ch, desc)
{
	ch->sendPrompt();
}

/** Parse a line of input and add a command to the execution queue
 */
void PlayerParser::parseLine(QString line)
{
	QString cline = line.simplifyWhiteSpace();
	QString cmdword = cline.section(' ', 0, 0);

	/* If they didn't input anything, just send the prompt along */
	if (cmdword.isEmpty())
	{
		_ch->sendPrompt();
		return;
	}
	
	/* Search for the command */
	koalamud::Command *cmd = maincmdtree->findandcreate(cmdword, _ch, true);
	
	/* If the command wasn't found, say we couldn't find it and move on */
	if (!cmd)
	{
		QString out;
		QTextOStream os(&out);
		os << _ch->getName() << ", that command is unknown." << endl;
		_ch->sendtochar(out);
		_ch->sendPrompt();
		return;
	} 
	
	/* Append cmd to the command queue */
	_ch->queueCommand(cmd, cline.section(' ', 1));
}

/** Build a player Creation parser
 */
PlayerCreationParser::PlayerCreationParser(ParseDescriptor *desc, QString name=NULL)
	: Parser(NULL, desc)
{
	QString out;
	QTextOStream os(&out);

	if (name.isEmpty())
	{
		/* No name has been provided yet. */
		curstate = STATE_GETNAME;
		os << endl << "Please choose a name, adventurer: ";
	} else {
		/* Make sure the name choosen isn't on the restricted list.  If it is,
		 * display an appropriate message and request a new name, otherwise skip
		 * right to requesting their characters last name.
		 */
		if (checkName(name))
		{
			_fname = name;
			curstate = STATE_GETLAST;
			os << endl << "Please choose a last name for your character: ";
		} else {
			os << endl << "That name is not allowed, please choose another name.";
			os << endl << "Please choose a name, adventurer: ";
			curstate = STATE_GETNAME;
		}
	}

	desc->send(out);
}

/** Parse input lines towards creating a new character */
void PlayerCreationParser::parseLine(QString line)
{
	QString cline = line.simplifyWhiteSpace();
	QString out;
	QTextOStream os(&out);

	switch (curstate)
	{
		case STATE_GETNAME:
			if (checkName(cline))
			{
				/* Check to see if 'cline' exists as a name */
				QString query;
				QTextOStream qos(&query);
				QSqlQuery q;
				qos << "select playerid from players where name = '" << cline << "';";
				if (q.exec(query) && q.numRowsAffected() == 0)
				{
					os << endl <<"Are you sure you want '" << cline
						 << "' for your name? (y/N)";
					curstate = STATE_CONFNAME;
					_fname = cline;
				} else {
					os << endl << "Sorry, that name is already in use." << endl;
					os << "Please choose a name, adventurer: ";
					curstate = STATE_GETNAME;
				}
			} else {
				os << endl << "That name is not allowed, please choose another name."
					 << endl << "Please choose a name, adventurer: ";
				curstate = STATE_GETNAME;
			}
			break;
		case STATE_CONFNAME:
			if (QString("yes").startsWith(cline.lower()))
			{
				curstate = STATE_GETLAST;
				os << endl << "Please enter a last name for your character: ";
			} else {
				os << endl << "Well then, what *do* you want to be known by? ";
				curstate = STATE_GETNAME;
				_fname.truncate(0);
			}
			break;
		case STATE_GETLAST:
			if (checkName(cline))
			{
				_lname = cline;
				os << endl << "So you wish to be known as " << _fname << " " << _lname
					 << ", correct? (y/N) ";
				curstate = STATE_CONFLAST;
			} else {
				os << endl << "That is not allowed as a last name, "
					 << "please choose another name." << endl;
				os << "Please choose a last name for yourself, " << _fname << ": ";
				curstate = STATE_GETLAST;
			}
			break;
		case STATE_CONFLAST:
			if (QString("yes").startsWith(cline.lower()))
			{
				curstate = STATE_GETPASS;
				os << endl << "Please choose a password to protect your character:";
			} else {
				os << endl << "Well then, what *do* you want your last name to be? ";
				curstate = STATE_GETLAST;
				_lname.truncate(0);
			}
			break;
		case STATE_GETPASS:
			_pass = cline;
			os << endl << "Please confirm your password: ";
			curstate = STATE_CONFPASS;
			break;
		case STATE_CONFPASS:
			if (_pass == cline)
			{
				curstate = STATE_GETEMAIL;
				os << endl <<
"Your email address is only used to send updates on system status," << endl <<
"upcoming events, and for password resets.  We will never sell your" << endl<<
"email address or send commercial email to your address." << endl <<
"Please enter your email address:";
			} else {
				os << endl << "Your password didn't match." << endl;
				os << endl << "Please choose a password to protect your character:";
			}
			break;
		case STATE_GETEMAIL:
			if (checkEmail(cline))
			{
				_email = cline;
				curstate = STATE_CONFEMAIL;
				os << endl << "You entered '" << _email 
					 << "' for your email address." << endl
					 << "Is this correct? (y/N) ";
			} else {
				os << endl << "That didn't look like a valid email address." << endl;
				os <<
"Your email address is only used to send updates on system status," << endl <<
"upcoming events, and for password resets.  We will never sell your" << endl<<
"email address or send commercial email to your address." << endl <<
"Please enter your email address:";
				curstate = STATE_GETEMAIL;
			}
			break;
		case STATE_CONFEMAIL:
			if (QString("yes").startsWith(cline.lower()))
			{
				/* phew, we got here and now we are all done (for now) */
				/* First - Create a database record for the new player */
				createDBRecord();
				os << endl
					 <<"Your character has been created and is ready for adventure."
				   << endl;
				_desc->send(out);
				_ch = new PlayerChar(_fname, _desc);
				_desc->setParser(new PlayerParser(_ch, _desc));
				return;
			} else {
				os << endl <<
"Your email address is only used to send updates on system status," << endl <<
"upcoming events, and for password resets.  We will never sell your" << endl<<
"email address or send commercial email to your address." << endl <<
"Please enter your email address:";
				curstate = STATE_GETEMAIL;
			}
			break;
	}

	if (!out.isEmpty())
	{
		_desc->send(out);
	}
}

/** Create the initial player record in the database
 * This gives something for the PlayerChar class to load while entering the
 * player into the game.
 * @note Since this is an internal function, we can't get here unless
 * everything is done for creation.  No need for checking variables.
 */
void PlayerCreationParser::createDBRecord(void)
{
	QString query;
	QTextOStream qos(&query);
	QSqlQuery q;

	qos << "insert into players (name, lastname, pass, email, created)" << endl
			<< "values" << endl
			<< "('" << _fname << "', '" << _lname << "', MD5('" << _pass << "'), '"
			<< _email << "', NOW());";
	q.exec(query);
}

}; /* end koalamud namespace */
