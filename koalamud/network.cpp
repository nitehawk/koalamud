
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

#include <qregexp.h>

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
	: QSocket(), outputEventPosted(false), sendcolor(false)
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

/** If there is not an unprocessed event for output, post one */
void Descriptor::notifyOutput(Char *ch)
{
	outputEventLock.acquire();
	if (!outputEventPosted)
	{
		outputEventPosted = true;
		QThread::postEvent(this, new CharOutputEvent(ch));
	}
	outputEventLock.release();
}

/** Connection was closed
 * Our connection closed.  SelfDestruct
 */
void Descriptor::closed(void)
{
	delete this;
}

/** Send data out to the network
 * This class will replace or strip color codes as needed
 */
void Descriptor::send(QString data)
{
	QTextStream os(this);
	char *dataout, *outpos;
	const char *datain = data.latin1();
	unsigned int count;
	unsigned int inlen = data.length();
	unsigned int pos = 0;
	const char *inpos = datain;
	/* Check to see if there are any color code markers - | is our marker */
	if ((count = data.contains("|")))
	{
		outpos = dataout = new char[inlen + count*8];
		bzero(dataout, inlen + count*8);

		while (pos <= inlen)
		{
			if (*inpos != '|')
			{
				*outpos++ = *inpos++;
				pos++;
			} else {
				inpos++;
				pos++;
				if (*inpos == '|')
				{
					*outpos++ = '|';
				} else if (sendcolor) {
					/* interpret a color code */
					switch(*inpos)
					{
						case 'x':
							strcpy(outpos,"\x1B[0;0m");
							outpos += 6;
							break;
						case 'l':
							strcpy(outpos,"\x1B[0;30m");
							outpos += 7;
							break;
						case 'r':
							strcpy(outpos,"\x1B[0;31m");
							outpos += 7;
							break;
						case 'g':
							strcpy(outpos,"\x1B[0;32m");
							outpos += 7;
							break;
						case 'y':
							strcpy(outpos,"\x1B[0;33m");
							outpos += 7;
							break;
						case 'b':
							strcpy(outpos,"\x1B[0;34m");
							outpos += 7;
							break;
						case 'm':
							strcpy(outpos,"\x1B[0;35m");
							outpos += 7;
							break;
						case 'c':
							strcpy(outpos,"\x1B[0;36m");
							outpos += 7;
							break;
						case 'w':
							strcpy(outpos,"\x1B[0;37m");
							outpos += 7;
							break;
						case 'L':
							strcpy(outpos,"\x1B[1;30m");
							outpos += 7;
							break;
						case 'R':
							strcpy(outpos,"\x1B[1;31m");
							outpos += 7;
							break;
						case 'G':
							strcpy(outpos,"\x1B[1;32m");
							outpos += 7;
							break;
						case 'Y':
							strcpy(outpos,"\x1B[1;33m");
							outpos += 7;
							break;
						case 'B':
							strcpy(outpos,"\x1B[1;34m");
							outpos += 7;
							break;
						case 'M':
							strcpy(outpos,"\x1B[1;35m");
							outpos += 7;
							break;
						case 'C':
							strcpy(outpos,"\x1B[1;36m");
							outpos += 7;
							break;
						case 'W':
							strcpy(outpos,"\x1B[1;37m");
							outpos += 7;
							break;
						default:
							*outpos++ = '|';
							*outpos++ = *inpos;
					}
				} else {
					if (!index("xlrgybcmwLRGYBCMW", *inpos))
					{
						*outpos++ = '|';
						*outpos++ = *inpos;
					}
				}
				inpos++;
				pos++;
			}
		}
		os << dataout;
		delete[] dataout;
	} else {
		os << datain;
	}
}

/** Handle incoming events */
bool Descriptor::event(QEvent *event)
{
	outputEventLock.acquire();
	static bool disconeventposted = false;
	if (event->type() == EVENT_CHAR_OUTPUT)
	{
		outputEventPosted = false;
		CharOutputEvent *ce = (CharOutputEvent *)event;
		if (ce->_ch->isDisconnecting())
		{
			if (disconeventposted)
			{
				outputEventLock.release();
				delete ce->_ch;
				delete this;
				return true;
			}
			disconeventposted = true;
			QThread::postEvent(this, new CharOutputEvent(ce->_ch));
			outputEventPosted = true;
			outputEventLock.release();
		} else {
		}
		outputEventLock.release();
		return true;
	}
	outputEventLock.release();
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
void ParseDescriptor::setParser(Parser *newparse, bool del=true)
{
	if (del)
		delete _parse;
	_parse=newparse;
}

}; /* end koalamud namespace */
