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

/* Predefine classes */
namespace koalamud {
	class Channel;
};

#include "char.hxx"
#include "cmd.hxx"

namespace koalamud
{
/** Communications channel
 * @note  This code will most likely need to be rewritten at some point to
 * account for the languages portion of the system.
 */
class Channel : public QObject
{
	/* NOTE:  right now there is no way to generate a list of players in the
	 * channel.  This may change if it looks necessary, but at this point, there
	 * are no plans to use the channel system on its own to provide player
	 * channels where member lists are useful */
	Q_OBJECT

	protected:
		/** Name of the channel */
		QString _name;
		/** Template used to send to channel members */
		QString _templateall;
		/** Template used to echo back to the sender */
		QString _templatesender;

	public:
		/** Setup a new channel */
		Channel(QString name, QString chantemplateall,
				QString chantemplatesender);
		/** Destroy a channel */
		~Channel();
		/** Join character to channel.  We don't char if the Char is a PC or NPC
		 */
		void joinchannel(Char *ch);
		/** Leave channel */
		void leavechannel(Char *ch);
		/** Get channel name */
		QString getName(void) { return _name; }
		/** Send a message to the channel */
		void sendtochannel(Char *, QString msg);

	signals:
		/** Relay message out to all channel members */
		void channelmessagesent(Char *sender, QString chantemplate,
				QString chantemplatesender, QString message);
		/** The channel has been deleted - remove from internal channel list */
		void channeldeleted(Channel *chan);
};

}; /* end koalamud namespace */

#ifdef KOALA_COMM_CXX
QDict<koalamud::Channel> channelmap(101, false);
#else
extern QDict<koalamud::Channel> channelmap;
#endif // KOALA_COMM_CXX

#endif //  KOALA_COMM_HXX
