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
#include "cmdtree.hxx"

/* New Command Stuff */
namespace koalamud {
	namespace commands {
/** Memstat command class */
class Memstat : public Command
{
	public:
		/** Pass through constructor */
		Memstat(K_PlayerChar *ch) : Command(ch) {}
		/** Run memstat command */
		virtual unsigned int run(QString cmd, QString args)
		{
			QString str;
			QTextOStream os(&str);

			os << *koalamud::PoolAllocator::instance() << endl;;

			_ch->sendtochar(str);
			return 0;
		}

};

/** Who command class */
class Who : public Command
{
	public:
		/** Pass through constructor */
		Who(K_PlayerChar *ch) : Command(ch) {}
		/** Run who command */
		virtual unsigned int run(QString cmd, QString args)
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
		Quit(K_PlayerChar *ch) : Command(ch) {}
		/** Run quit command */
		virtual unsigned int run(QString cmd, QString args)
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
		}

		/** Handle command object creations */
		virtual Command *create(unsigned int id, K_PlayerChar *ch)
		{
			switch (id)
			{
				case 1:
					return new koalamud::commands::Memstat(ch);
				case 2:
					return new koalamud::commands::Quit(ch);
				case 3:
					return new koalamud::commands::Who(ch);
			}
			return NULL;
		}
};

/* The factories will only ever have one instance, and it isn't explicitly
 * used after creation */
Cmd_CPP_CommandFactory Cmd_CPP_CommandFactoryInstance;

}; /* end namespace koalamud */
