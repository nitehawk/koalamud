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

namespace koalamud {
	namespace commands {

/** Tell command class */
class Tell : public Command
{
	public:
		/** Pass through constructor */
		Tell(K_PlayerChar *ch) : Command(ch) {}

		/** Run tell command */
		virtual unsigned int run(QString cmd, QString args)
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
			K_PlayerChar *tellto = connectedplayermap[to];
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
			tos << _ch->getName(tellto) << " tells you, '" << msg << "'" << endl;
			tellto->sendtochar(tstr);

			/* Construct message to sender */
			QString sstr;
			QTextOStream sos(&sstr);
			sos << "You tell " << tellto->getName(_ch) << ", '" << msg << "'" << endl;
			_ch->sendtochar(sstr);
			return 0;
		}
};

/** Gossip command class */
class Gossip : public Command
{
	public:
		/** Pass through constructor */
		Gossip(K_PlayerChar *ch) : Command(ch) {}

		/** Run gossip command */
		virtual unsigned int run(QString cmd, QString args)
		{
			KoalaChannel *goschan = channelmap["gossip"];
			if (goschan == NULL)
			{
				return 1;
			}

			goschan->sendtochannel(_ch, args);
			return 0;
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
				new KoalaChannel("gossip", "%sender% gossips, '%message%'",
							"%sender% gossip, '%message%'");

				maincmdtree->addcmd("gossip", this, 1);
				maincmdtree->addcmd("tell", this, 2);
			}

			/** Handle Command object creation */
			virtual Command *create(unsigned int id, K_PlayerChar *ch)
			{
				switch (id)
				{
					case 1:
						return new koalamud::commands::Gossip(ch);
					case 2:
						return new koalamud::commands::Tell(ch);
				}
				return NULL;
			}
	};

	Comm_CPP_CommandFactory Comm_CPP_CommandFactoryInstance;
}; /* end koalamud namespace */

/* Channel class implementation */
KoalaChannel::KoalaChannel(QString name, QString chantemplateall,
		QString chantemplatesender)
	: QObject(NULL, name), _name(name), _templateall(chantemplateall),
		_templatesender(chantemplatesender)
{
	/* Put channel into the channel map */
	channelmap.insert(_name, this);
}

KoalaChannel::~KoalaChannel(void)
{
	channelmap.remove(_name);
}

void KoalaChannel::joinchannel(K_PlayerChar *ch)
{
	connect(this,
		SIGNAL(channelmessagesent(K_PlayerChar *, QString, QString, QString)),
		ch,
		SLOT(channelsendtochar(K_PlayerChar *, QString, QString, QString)));
		
	connect(this, SIGNAL(channeldeleted(KoalaChannel *)),
		ch, SLOT(channeldeleted(KoalaChannel *)));
}

void KoalaChannel::leavechannel(K_PlayerChar *ch)
{
	disconnect(ch);
}

void KoalaChannel::sendtochannel(K_PlayerChar *ch, QString msg)
{
	/* For the moment, just emit the signal, no processing */
	emit channelmessagesent(ch, _templateall, _templatesender, msg);
}
