
/***************************************************************\
*                        KoalaMud Gen 2                         *
*    Copyright (c) 2002 First Step Internet Services, Inc.      *
*                     All Rights Reserved                       *
*        Distributed under the terms of the FSI License         *
* See file LICENSE in the root of this package for information  *
\***************************************************************/
/***************************************************************\
*	Module:
* Description:
* Classes:
\***************************************************************/

#define KOALA_NETWORK_CXX "%A%"

#include "main.hxx"
#include "logging.hxx"
#include "network.hxx"
#include "parser.hxx"
#include "event.hxx"

namespace koalamud {
/** Initialize a listener object
 * Setup a port listener and update the gui status
 */
Listener::Listener(unsigned int port, porttype_t newporttype = GAMESERVER)
: QServerSocket(port), _type(newporttype), _port(port)
{
    /* Create a list item if the gui is active */
    if (srv->usegui())
    {
			QString portstr;
			portstr.setNum(port);
			ListenStatusItem = new QListViewItem(srv->statwin()->ListenStatusList,
									portstr, "Online");
			srv->statwin()->updatelistencount();
    }

	QString str;
	QTextOStream os(&str);
	os << "New ";

	switch (_type)
	{
		case GAMESERVER:
			os << "game server";
			break;
	}

	os << " started on port #" << port;
	Logger::msg(str, Logger::LOG_NOTICE);
}

/** Destroy a listener object */
Listener::~Listener()
{
	/* delete the list item for the port */
	delete ListenStatusItem;
	if (srv->usegui())
	{
	    srv->statwin()->updatelistencount();
	}
	/* remove ourself from the list of listeners */
}

/** Handle a newly accepted connection and bring it into the game world */
void Listener::newConnection(int socket)
{
	ParseDescriptor *desc;

	/* FIXME:  This should be based on the listener type and in most cases
	 * simply create a descriptor - not a playerchar object. */
	switch (_type)
	{
		case GAMESERVER:
			desc = new ParseDescriptor(socket);
			desc->setParser(new PlayerLoginParser(NULL, desc));
			break;
	}
}

/** Construct a Descriptor object */
Descriptor::Descriptor(int sock)
	: QSocket()
{
	connect(this, SIGNAL(readyRead()), SLOT(readClient()));
	connect(this, SIGNAL(connectionClosed()), SLOT(closed()));
	setSocket(sock);

	QString str;
	QTextOStream os(&str);
	os << "New connection from " << peerAddress().toString();
	Logger::msg(str);
}

/** Read data from the socket
 * This version echos all incoming data back out to the network
 */
void Descriptor::readClient(void)
{
	QTextStream os(this);
	while ( canReadLine() )
	{
		os << "ECHO: " << readLine();
	}
	os << endl;
}

/** Connection was closed
 * Our connection closed.  SelfDestruct
 */
void Descriptor::closed(void)
{
	delete this;
}

/** Send data out to the network
 * No additional processing is done on the data (subclasses might do some
 * additional stuff here.
 */
void Descriptor::send(QString data)
{
	QTextStream os(this);
	os << data;
}

/** Handle incoming events */
bool Descriptor::event(QEvent *event)
{
	static bool disconeventposted = false;
	if (event->type() == EVENT_CHAR_OUTPUT)
	{
		CharOutputEvent *ce = (CharOutputEvent *)event;
		if (ce->_ch->isDisconnecting())
		{
			if (disconeventposted)
			{
				delete ce->_ch;
				delete this;
				return true;
			}
			disconeventposted = true;
			QThread::postEvent(this, new CharOutputEvent(ce->_ch));
		} else {
		}
		return true;
	}
	return false;
}

/** Construct a Descriptor object
 * @param sock identifier of connected socket.
 * @param parser Pointer to parser object to start system with
 */
ParseDescriptor::ParseDescriptor(int sock, Parser *parser = NULL)
	: Descriptor(sock), _parse(parser)
{
}

/** Read data from the socket
 * This version passes each line read into the currently connected parser or
 * discards them if not connected.
 */
void ParseDescriptor::readClient(void)
{
	while (canReadLine())
	{
		if (_parse)
		{
			_parse->parseLine(readLine());
		} else {
			readLine();
		}
	}
}

/** Destroy a descriptor */
ParseDescriptor::~ParseDescriptor(void)
{
	delete _parse;
}

/** Attach a new parser and delete the old parser */
void ParseDescriptor::setParser(Parser *newparse)
{
	delete _parse;
	_parse=newparse;
}

}; /* end koalamud namespace */
