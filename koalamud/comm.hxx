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

#ifndef KOALA_COMM_HXX
#define KOALA_COMM_HXX "%A%"

#include <qobject.h>
#include <qdict.h>

class KoalaChannel;
#include "cmd.hxx"

/* Channel class */
class KoalaChannel : public QObject
{
	/* NOTE:  right now there is no way to generate a list of players in the
	 * channel.  This may change if it looks necessary, but at this point, there
	 * are no plans to use the channel system on its own to provide player
	 * channels where member lists are useful */
	Q_OBJECT

	protected:
		QString _name;
		QString _templateall;
		QString _templatesender;

	public:
		KoalaChannel(QString name, QString chantemplateall,
				QString chantemplatesender);
		~KoalaChannel();
		void joinchannel(K_PlayerChar *ch);
		void leavechannel(K_PlayerChar *ch);
		QString getName(void) { return _name; }
		void sendtochannel(K_PlayerChar *, QString msg);

	signals:
		/* When joinchannel is called, the player is connected to the
		 * channelmessagesent and channeldeleted signals.  channeldeleted allows
		 * us to ensure that we don't attempt to display dead channels if the
		 * player requests a list of channels they are on */
		void channelmessagesent(K_PlayerChar *sender, QString chantemplate,
				QString chantemplatesender, QString message);
		void channeldeleted(KoalaChannel *chan);
};

#ifdef KOALA_COMM_CXX
QDict<KoalaChannel> channelmap(101, false);
#else
extern QDict<KoalaChannel> channelmap;
#endif // KOALA_COMM_CXX

#endif //  KOALA_COMM_HXX
