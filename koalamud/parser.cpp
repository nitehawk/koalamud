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
		/* Eventually we will store a variety of welcome screens in the database
		 * and choose one to send out.  For now, just print a simple welcome. */
		QString str;
		QTextOStream os(&str);

		os << "Welcome to " << srv->versionstring() << endl;
		os << "By what name are you known? ";
		_desc->send(str);
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
				os << "Are you sure you want to create a new character?";
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
						os << "Enter your password: ";
					} else {
						state = STATE_CONFNAME;
						os << "That player does not exist, would you like to create"
							 << "a new character?";
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
					os << "I'm sorry, that password is incorrect." << endl
						 << "By what name are you known? ";
					state = STATE_GETNAME;
				}
			}
			break;
		case STATE_CONFNAME:
			if (QString("yes").startsWith(sline.lower()))
			{
				/* This will transistion to the character creation code */
				os << "New character creation is temporarily disabled." << endl
					 << "What is your name?";
				state = STATE_GETNAME;
			} else {
				state = STATE_GETNAME;
				os << "Ok then, What is your name?";
			}
			break;
		case STATE_CONFNEW:
			if (QString("yes").startsWith(sline.lower()))
			{
				/* This will transistion to the character creation code */
				os << "New character creation is temporarily disabled." << endl
					 << "What is your name?";
				state = STATE_GETNAME;
			} else {
				state = STATE_GETNAME;
				os << "Ok then, What is your name?";
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
	
}; /* end koalamud namespace */
