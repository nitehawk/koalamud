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

KOALACMD(cmd_tell)
{
	QString to = args.section(' ', 0, 0);
	QString msg = args.section(' ', 1);

	/* Check that we have parameters */
	if (to.isEmpty())
	{
		QString str;
		QTextOStream os(&str);
		os << "Who would you like to tell something to?" << endl;
		ch->sendtochar(str);
		return;
	} else if (msg.isEmpty())
	{
		QString str;
		QTextOStream os(&str);
		os << "What would you like to tell " << to << "?" << endl;
		ch->sendtochar(str);
		return;
	}

	/* First up we need to get a pointer to the player */
	K_PlayerChar *tellto = connectedplayermap[to];
	if (tellto == NULL)
	{
		QString str;
		QTextOStream os(&str);
		os << "That player is not logged on." << endl;
		ch->sendtochar(str);
		return;
	} else if (ch == tellto)
	{
		QString str;
		QTextOStream os(&str);
		os << "You try to tell yourself something." << endl;
		ch->sendtochar(str);
		return;
	}

	/* Construct message to target */
	QString tstr;
	QTextOStream tos(&tstr);
	tos << ch->getName(tellto) << " tells you, '" << msg << "'" << endl;
	tellto->sendtochar(tstr);

	/* Construct message to sender */
	QString sstr;
	QTextOStream sos(&sstr);
	sos << "You tell " << tellto->getName(ch) << ", '" << msg << "'" << endl;
	ch->sendtochar(sstr);
}

KOALACMD(cmd_gossip)
{
	KoalaChannel *goschan = channelmap["gossip"];
	if (goschan == NULL)
	{
		return;
	}

	goschan->sendtochannel(ch, args);
}

/* Load communications commands into the dictionary */
/* NOTE:  We also setup the comm channels here */
void initcommcmddict(void)
{
	cmddict.insert(QString("tell"), new cmdentry_t("tell", cmd_tell));
	cmddict.insert(QString("gossip"), new cmdentry_t("gossip", cmd_gossip));
	cmddict.insert(QString("gos"), new cmdentry_t("gossip", cmd_gossip));
	cmddict.insert(QString("goss"), new cmdentry_t("gossip", cmd_gossip));

	new KoalaChannel("gossip", "%sender% gossips, '%message%'",
			"%sender% gossip, '%message%'");
}

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
