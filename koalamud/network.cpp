
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
#include <qhostaddress.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include "main.hxx"
#include "logging.hxx"
#include "network.hxx"
#include "parser.hxx"
#include "event.hxx"

namespace koalamud {

/** Initialize a network socket */
Socket::Socket(int sock = 0)
	: _sock(sock), _closeme(false)
{
	/* If there is no socket, create one */
	if (_sock == 0)
	{
		struct protoent *protoentry = NULL;
		protoentry = getprotobyname("tcp");
		_sock = socket(AF_INET, SOCK_STREAM, protoentry->p_proto);
	}

	/* Set socket options */
	{
		/* Set Linger */
		struct linger l;
		l.l_onoff = 0;
		l.l_linger = 0;
		setsockopt(_sock, SOL_SOCKET, SO_LINGER, &l, sizeof(l));
	}

	{
		/* Set non blocking */
		int sockflags;
		int result;
		sockflags = fcntl(_sock, F_GETFL, 0);
		sockflags |= O_NONBLOCK;
		result = fcntl(_sock, F_SETFL, sockflags);
	}

	{
		/* Set reuse addr */
		int optval = 1;
		setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	}

	srv->addSocktoList(this);
}

/** Destroy a network socket - remove it from the list */
Socket::~Socket(void)
{
	srv->removeSockfromList(this);
}

/** Initialize a listener object
* Setup a port listener and update the gui status
*/
Listener::Listener(unsigned int port, porttype_t newporttype = GAMESERVER)
: _type(newporttype), _port(port)
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

	/* Finish setting up bind to socket */
	struct sockaddr_in addr;
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htons(INADDR_ANY);

	/* Bind away */
	bind(_sock, (struct sockaddr *)&addr, sizeof(addr));
	listen(_sock, 5);

	/* Log a message */
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

/** Dispatch a read message.
 * In listener case, we accept a connection and call newConnection with it.
 */
void Listener::dispatchRead(void)
{
	struct sockaddr addr;
	socklen_t slen = sizeof(struct sockaddr);

	int newsock = accept(_sock, &addr, &slen);
	newConnection(newsock);
}

/** Handle a newly accepted connection and bring it into the game world */
void Listener::newConnection(int socket)
{
	ParseDescriptor *desc;

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
	: Socket(sock), sendcolor(false), inBuffer(4096), outBuffer(4096)
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(struct sockaddr_in);

	memset(&addr, 0, len);
	getpeername(_sock, (struct sockaddr *)&addr, &len);
	
	QString str;
	QTextOStream os(&str);
	os << "New connection from "
		 << QHostAddress(ntohl(addr.sin_addr.s_addr)).toString();
	Logger::msg(str);
}

/** Dispatch read read events for Descriptors */
void Descriptor::dispatchRead(void)
{
	/* Read data into the buffer from the socket, then pass it over to
	 * readClient */
	char *start = inBuffer.getTail();
	int maxread = inBuffer.getFree();
	int numread = read(_sock, start, maxread);
	inBuffer.externDatain(numread);

	if (numread == 0)
	{
		delete this;
		return;
	}

	readClient();
}

/** Handle write events for descriptors
 * Take a chunk of data from the outBuffer and send it on to the socket
 * If the buffer is empty when we are done and we are closing, then close the
 * socket and selfdestruct
 */
void Descriptor::doWrite(void)
{
	char buf[512];
	int len = outBuffer.getData(buf, 512);

	write(_sock, buf, len);

	if (_closeme && outBuffer.isEmpty())
	{
		close(_sock);
		delete this;
	}
}

/** Check to see if there is data pending in the output buffer */
bool Descriptor::isDataPending(void)
{
	return !outBuffer.isEmpty();
}

/** Read data from the socket
 * This version echos all incoming data back out to the network
 */
void Descriptor::readClient(void)
{
	QString out;
	QTextOStream os(&out);
	char * input;
	while ((input = inBuffer.getLine())!= NULL)
	{
		os << "ECHO: " << input;
		free(input);
	}
	os << endl;
	send(out);
}

/** Send data out to the network
 * This class will replace or strip color codes as needed
 */
void Descriptor::send(QString data)
{
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
		++outpos = '\0';
		/* Put this on the output buffer */
		char *bufpos = outBuffer.getTail();
		int outlen = strlen(dataout);
		if (outlen > outBuffer.getFree())
			outlen = outBuffer.getFree();
		strncpy(bufpos, dataout, outlen);
		outBuffer.externDatain(outlen);
		delete[] dataout;
	} else {
		/* Put this on the output buffer */
		char *bufpos = outBuffer.getTail();
		int outlen = inlen;
		if (outlen > outBuffer.getFree())
			outlen = outBuffer.getFree();
		strncpy(bufpos, datain, outlen);
		outBuffer.externDatain(outlen);
	}
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
	char *input;
	while ((input = inBuffer.getLine()) != NULL )
	{
		if (_parse)
		{
			_parse->parseLine(QString(input));
		} else {
		}
		free(input);
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
