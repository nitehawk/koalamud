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

#include "main.hxx"
#include "exception.hxx"
#include "logging.hxx"
#include "cmd.hxx"
#include "cmdtree.hxx"
#include "playerchar.hxx"
#include "room.hxx"

/* New Command Stuff */
namespace koalamud {
	namespace commands {
/** Memstat command class */
class Memstat : public Command
{
	public:
		/** Pass through constructor */
		Memstat(Char *ch) : Command(ch) {}
		/** Run memstat command */
		virtual unsigned int run(QString args)
		{
			QString str;
			QTextOStream os(&str);

			os << *koalamud::PoolAllocator::instance() << endl;;

			_ch->sendtochar(str);
			return 0;
		}

		/** Restricted access command. */
		virtual bool isRestricted(void) const { return true;}

		/** Command Groups */
		virtual QStringList getCmdGroups(void) const
		{
			QStringList gl;
			gl << "Implementor" << "Coder";
			return gl;
		}

		/** Get command name for individual granting */
		virtual QString getCmdName(void) const { return QString("memstat"); }

};

/** Who command class */
class Who : public Command
{
	public:
		/** Pass through constructor */
		Who(Char *ch) : Command(ch) {}
		/** Run who command */
		virtual unsigned int run(QString args)
		{
			QString str;
			QTextOStream os(&str);

			/* Grab an iterator to the player list and loop through it */
			playerlistiterator_t pli(connectedplayerlist);
			Char *cur;
			os << "The following players are online: " << endl;
			while ((cur = pli.current()) != NULL)
			{
				++pli;
				/* Later we'll want to display more information as well as check
				 * imm invisibility and a lot of other stuff before displaying someone
				 */
				os << cur->getName(_ch) << endl;
			}

			_ch->sendtochar(str);
			return 0;
		}

};

/** Quit command class */
class Quit : public Command
{
	public:
		/** Pass through constructor */
		Quit(Char *ch) : Command(ch) {}
		/** Run quit command */
		virtual unsigned int run(QString args)
		{
			QString str;
			QTextOStream os(&str);

			os << "Thank you for playing on KoalaMud." << endl;
			os << "Please return soon." << endl;
			_ch->sendtochar(str);

			_ch->setdisconnect(true);
			return 0;
		}

};

/** shutdown command class */
class Shutdown : public Command
{
	public:
		/** Pass through constructor */
		Shutdown(Char *ch) : Command(ch) {}
		/** Run shutdown command */
		virtual unsigned int run(QString args)
		{
			QString str;
			QTextOStream os(&str);

			os << "KoalaMud Server shutting down." << endl;
			_ch->sendtochar(str);

			srv->shutdown(0);
			return 0;
		}

		/** Restricted access command. */
		virtual bool isRestricted(void) const { return true;}

		/** Command Groups */
		virtual QStringList getCmdGroups(void) const
		{
			QStringList gl;
			gl << "Implementor" << "Coder";
			return gl;
		}

		/** Get command name for individual granting */
		virtual QString getCmdName(void) const { return QString("shutdown"); }
};

/** grant command class */
class Grant : public Command
{
	public:
		/** Pass through constructor */
		Grant(Char *ch) : Command(ch) {}
		/** Run grant command
		 * grant [add|remove|block|groupadd|grouprem] [cmd or group] [player]
		 * We expect three strings as arguments.  For simplicity, we'll parse
		 * these ourself instead of going down another class level. */
		virtual unsigned int run(QString args)
		{
			QSqlQuery q;
			QString str;
			QTextOStream os(&str);
			QString action = args.section(' ', 0, 0);
			QString cmdgroup = args.section(' ', 1, 1);
			QString player = args.section(' ', 2, 2);
			unsigned int playerid = 0;
			unsigned int groupnum = 0;
			enum {act_add, act_remove, act_block, act_groupadd, act_grouprem,
					act_null} act = act_null;

			/* Check our action and make sure its valid */
			if (action.length() > 0)
			{
				if (QString("add").startsWith(action.lower()))
					act = act_add;
				if (QString("remove").startsWith(action.lower()))
					act = act_remove;
				if (QString("block").startsWith(action.lower()))
					act = act_block;
				if (QString("groupadd").startsWith(action.lower()))
					act = act_groupadd;
				if (QString("groupremove").startsWith(action.lower()))
					act = act_grouprem;
			}
			if (act == act_null)
			{
				os << "You didn't specify a valid action.  See help grant." << endl;
				_ch->sendtochar(str);
				return 1;
			}

			/* Validate command or group */
			if (act == act_add || act == act_remove || act == act_block)
			{
				/* Search maincmdtable and immcmdtable */
				if (maincmdtree->find_full(cmdgroup) == NULL
					/* && immcmdtree->find_full(cmdgroup) == NULL */)
				{
					os << "You didn't specify a valid command." << endl;
					_ch->sendtochar(str);
					return 1;
				}
			} else {
				/* Lookup groupid in database */
				QString query;
				QTextOStream qos(&query);

				qos << "select gid from commandgroup where gname = '" << cmdgroup
						<< "';";
				if (q.exec(query))
				{
					if (q.numRowsAffected() > 0)
					{
						q.next();
						groupnum = q.value(0).toInt();
					}
				}
				if (groupnum == 0)
				{
					os << "You didn't specify a valid command group." << endl;
					_ch->sendtochar(str);
					return 1;
				}
			}

			/* Check to see if player is valid */
			if (player.length() > 1)
			{
				/* Build a query and get the playerid from the database */
				QString query;
				QTextOStream qos(&query);

				qos << "select playerid from players where name = '" << player
						<< "';";
				if (q.exec(query))
				{
					if (q.numRowsAffected() > 0)
					{
						q.next();
						playerid = q.value(0).toInt();
					}
				}

			} else {
				os << "You didn't specify a player to change permissions on." << endl;
				_ch->sendtochar(str);
				return 1;
			}
			if (playerid == 0)
			{
				os << "That player does not exist." << endl;
				_ch->sendtochar(str);
				return 1;
			}

			/* Now that we are to this point, we just need to run the query to
			 * change permissions as needed */
			QString query;
			QTextOStream qos(&query);
			switch (act)
			{
				case act_add:
					qos << "replace into cmdperm (playerid, cmdname, allowed)" << endl
							<< "values (" << playerid << ", '" << cmdgroup << "', 'yes');";
					q.exec(query);
					os << player << " has been given permission to command " << cmdgroup
						 << "." << endl;
					break;
				case act_remove:
					qos << "delete from cmdperm where playerid = " << playerid << endl
							<< "and cmdname = '" << cmdgroup << "';";
					q.exec(query);
					os << player << " has been been removed from command " << cmdgroup
						 << "." << endl;
					break;
				case act_block:
					qos << "replace into cmdperm (playerid, cmdname, allowed)" << endl
							<< "values (" << playerid << ", '" << cmdgroup << "', 'no');";
					q.exec(query);
					os << player << " has been been blocked from command " << cmdgroup
						 << "." << endl;
					break;
				case act_groupadd:
					qos << "replace into groupmem (playerid, groupid) values" << endl
							<< "(" << playerid << ", " << groupnum << ");";
					q.exec(query);
					os << player << " has been added to group " << cmdgroup
						 << "." << endl;
					break;
				case act_grouprem:
					qos << "delete from groupmem where playerid = " << playerid << endl
							<< "and groupid = " << groupnum << ";";
					q.exec(query);
					os << player << " has been removed from group " << cmdgroup
						 << "." << endl;
					break;
				default:
					break;
			}

			_ch->sendtochar(str);

			return 0;
		}

		/** Restricted access command. */
		virtual bool isRestricted(void) const { return true;}

		/** Command Groups */
		virtual QStringList getCmdGroups(void) const
		{
			QStringList gl;
			gl << "Implementor";
			return gl;
		}

		/** Get command name for individual granting */
		virtual QString getCmdName(void) const { return QString("grant"); }
};

/** Look command class */
class Look : public Command
{
	public:
		/** Pass through constructor */
		Look(Char *ch) : Command(ch) {}
		/** Run Look command */
		virtual unsigned int run(QString args)
		{
			QString str;
			QTextOStream os(&str);

			os << _ch->getRoom()->displayRoom(_ch);
			_ch->sendtochar(str);

			return 0;
		}

};

	}; /* end namespace command */

/** Command Factory for cmd.cpp */
class Cmd_CPP_CommandFactory : public CommandFactory
{
	public:
		/** Register our commands */
		Cmd_CPP_CommandFactory(void)
			: CommandFactory()
		{
			maincmdtree->addcmd("memstat", this, 1);
			maincmdtree->addcmd("quit", this, 2);
			maincmdtree->addcmd("who", this, 3);
			maincmdtree->addcmd("shutdown", this, 4);
			maincmdtree->addcmd("grant", this, 5);
			maincmdtree->addcmd("look", this, 6);
		}

		/** Handle command object creations */
		virtual Command *create(unsigned int id, Char *ch)
		{
			switch (id)
			{
				case 1:
					return new koalamud::commands::Memstat(ch);
				case 2:
					return new koalamud::commands::Quit(ch);
				case 3:
					return new koalamud::commands::Who(ch);
				case 4:
					return new koalamud::commands::Shutdown(ch);
				case 5:
					return new koalamud::commands::Grant(ch);
				case 6:
					return new koalamud::commands::Look(ch);
			}
			return NULL;
		}
};

/** Command factory for cmd.cpp module.  */
Cmd_CPP_CommandFactory Cmd_CPP_CommandFactoryInstance;

/** This acts as the entry point for running commands.  It does all of the
 * permissions checking to make sure that the character has permission to run
 * the command, and runs the command if they do.
 */
unsigned int Command::runCmd(QString args)
	throw (koalamud::exceptions::cmdpermdenied)
{
	QSqlQuery q;

	if (_overrideperms)
		return run(args);
	
	/* Check if the command is restricted */
	if (!isRestricted())
		return run(args);

	/* We need to verify that ch has permission to run this command.
	 * First we need to check if they have explicitly been granted or denied
	 * permission to a command.  If that check turns up negative, we check to
	 * see if they are in one of the command groups this command is available
	 * to. */
	QString cmdname = getCmdName();
	if (cmdname.length() > 1)
	{
		/* Do a db lookup */
		QString query;
		QTextOStream qos(&query);

		qos << "Select cp.allowed from cmdperm as cp, players as p" << endl;
		qos << "where cp.playerid = p.playerid and" << endl;
		qos << "p.name = '" << _ch->getName() << "' and " << endl;
		qos << "cp.cmdname = '" << cmdname << "';";
		if (q.exec(query))
		{
			if (q.numRowsAffected())
			{
				q.next();
				if (q.value(0).toString() == "yes")
				{
					return run(args);
				} else if (q.value(0).toString() == "no")
				{
					throw koalamud::exceptions::cmdpermdenied();
				}
			}
		}
	}

	/* Check group permission */
	QStringList gl = getCmdGroups();
	QString grpwhere = gl.join("' or g.gname = '");
	if (grpwhere.length() > 1)
	{
		QString query;
		QTextOStream qos(&query);
		qos << "select gm.playerid, gm.groupid" << endl;
		qos << "from commandgroup as g, players as p, groupmem as gm" << endl;
		qos << "where gm.playerid = p.playerid and" << endl;
		qos << "p.name = '" << _ch->getName() << "' and" << endl;
		qos << "gm.groupid = g.gid and" << endl;
		qos << "(g.gname = '" << grpwhere << "');";

		if (q.exec(query))
			if (q.numRowsAffected())
				return run(args);
	}

	/* Add any additional permissions checks here. */
	
	/* If we get to here, deny permission */
	throw koalamud::exceptions::cmdpermdenied();
}


}; /* end namespace koalamud */
