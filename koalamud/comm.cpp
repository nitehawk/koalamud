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

#define KOALA_COMM_CXX "%A%"

#include "comm.hxx"
#include "playerchar.hxx"
#include "cmdtree.hxx"
#include "room.hxx"

namespace koalamud {
	namespace commands {

/** Tell command class */
class Tell : public Command
{
	public:
		/** Pass through constructor */
		Tell(Char *ch) : Command(ch) {}

		/** Run tell command */
		virtual unsigned int run(QString args)
		{
			QString to = args.section(' ', 0, 0);
			QString msg = args.section(' ', 1);

			/* Check that we have parameters */
			if (to.isEmpty())
			{
				QString str;
				QTextOStream os(&str);
				os << "Who would you like to tell something to?" << endl;
				_ch->sendtochar(str);
				return 1;
			} else if (msg.isEmpty())
			{
				QString str;
				QTextOStream os(&str);
				os << "What would you like to tell " << to << "?" << endl;
				_ch->sendtochar(str);
				return 1;
			}

			/* First up we need to get a pointer to the player */
			Char *tellto = connectedplayermap[to];
			if (tellto == NULL)
			{
				QString str;
				QTextOStream os(&str);
				os << "That player is not logged on." << endl;
				_ch->sendtochar(str);
				return 1;
			} else if (_ch == tellto)
			{
				QString str;
				QTextOStream os(&str);
				os << "You try to tell yourself something." << endl;
				_ch->sendtochar(str);
				return 1;
			}

			/* Construct message to target */
			QString tstr;
			QTextOStream tos(&tstr);
			tos << endl << "|R" << _ch->getName(tellto) << " tells you, '"
					<< msg << "|R'|x" << endl;
			tellto->sendtochar(tstr);
			tellto->sendPrompt();

			/* Construct message to sender */
			QString sstr;
			QTextOStream sos(&sstr);
			sos << "|RYou tell " << tellto->getName(_ch) << ", '" << msg
					<< "|R'|x" << endl;
			_ch->sendtochar(sstr);
			return 0;
		}
};

/** Gossip command class */
class Gossip : public Command
{
	public:
		/** Pass through constructor */
		Gossip(Char *ch) : Command(ch) {}

		/** Run gossip command */
		virtual unsigned int run(QString args)
		{
			/* Later this should scan for color codes as well */
			if (args.length() == 0)
			{
				QString out;
				QTextOStream os(&out);
				os << endl
					 << "Yes, gossip if you must, but what gossip do you want to share?"
					 << endl;
				_ch->sendtochar(out);
				return 1;
			}

			Channel *goschan = channelmap["gossip"];
			if (goschan == NULL)
			{
				return 1;
			}

			goschan->sendtochannel(_ch, args);
			return 0;
		}
};

/** Say command class */
class Say : public Command
{
	public:
		/** Pass through constructor */
		Say(Char *ch) : Command(ch) {}

		/** Run Say command */
		virtual unsigned int run(QString args)
		{
			/* Later this should scan for color codes as well */
			if (args.length() == 0)
			{
				QString out;
				QTextOStream os(&out);
				os << endl
					 << "What did you want to say?"
					 << endl;
				_ch->sendtochar(out);
				return 1;
			}

			if (_ch->getRoom())
			{
				QString fromstr;
				QTextOStream os(&fromstr);
				os << endl << "|g%sender% say, |c'%message%'|x" << endl;
				QString tostr;
				QTextOStream os3(&tostr);
				os3 << endl << "|g%sender% says, |c'%message%'|x" << endl;
				_ch->getRoom()->sendToRoom(_ch, NULL, args,
						fromstr, tostr, tostr);
			}
		}
};

	}; /* end commands namespace */
	
	/** Comm Module Command Factory */
	class Comm_CPP_CommandFactory : public CommandFactory
	{
		public:
			/** Register our commands and create gossip channel */
			Comm_CPP_CommandFactory(void) : CommandFactory()
			{
				new Channel("gossip", "|Y%sender% gossips, '%message%|Y'|x",
							"|Y%sender% gossip, '%message%|Y'|x");

				maincmdtree->addcmd("gossip", this, 1);
				maincmdtree->addcmd("tell", this, 2);
				maincmdtree->addcmd("say", this, 3);
			}

			/** Handle Command object creation */
			virtual Command *create(unsigned int id, Char *ch)
			{
				switch (id)
				{
					case 1:
						return new koalamud::commands::Gossip(ch);
					case 2:
						return new koalamud::commands::Tell(ch);
					case 3:
						return new koalamud::commands::Say(ch);
				}
				return NULL;
			}
	};

	/** Command factory instance for comm.cpp */
	Comm_CPP_CommandFactory Comm_CPP_CommandFactoryInstance;

/* Channel class implementation */
Channel::Channel(QString name, QString chantemplateall,
		QString chantemplatesender)
	: QObject(NULL, name), _name(name), _templateall(chantemplateall),
		_templatesender(chantemplatesender)
{
	/* Put channel into the channel map */
	channelmap.insert(_name, this);
}

Channel::~Channel(void)
{
	channelmap.remove(_name);
}

void Channel::joinchannel(Char *ch)
{
	connect(this,
		SIGNAL(channelmessagesent(Char *, QString, QString, QString)),
		ch,
		SLOT(channelsendtochar(Char *, QString, QString, QString)));
		
	connect(this, SIGNAL(channeldeleted(Channel *)),
		ch, SLOT(channeldeleted(Channel *)));
}

void Channel::leavechannel(Char *ch)
{
	disconnect(ch);
}

void Channel::sendtochannel(Char *ch, QString msg)
{
	/* For the moment, just emit the signal, no processing */
	emit channelmessagesent(ch, _templateall, _templatesender, msg);
}

}; /* end koalamud namespace */
