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

#define KOALA_LOGGING_CXX "%A%"

#include <qsqlquery.h>
#include <qsqlerror.h>
#include <qdatetime.h>
#include <qregexp.h>

#include "logging.hxx"
#include "cmd.hxx"
#include "cmdtree.hxx"

namespace koalamud {

/** Log a message to the database
 * This does the actual work of logging a message into the database and
 * sending the appropriate signal to send the message back out to listening
 * players, and logging to the console if we are not running in the
 * background.
 * @param lm message to log
 * @param sev message severity
 */
void Logger::imsg(QString lm, log_lev sev = LOG_INFO)
{
	QSqlQuery q;
	/* Get severity text */
	QString sevstring;
	switch (sev)
	{
		case LOG_FATAL: sevstring = "FATAL"; break;
		case LOG_SEVERE: sevstring = "Severe"; break;
		case LOG_CRITICAL: sevstring = "Critical"; break;
		case LOG_ERROR: sevstring = "Error"; break;
		case LOG_WARNING: sevstring = "Warning"; break;
		case LOG_NOTICE: sevstring = "Notice"; break;
		case LOG_INFO: sevstring = "Info"; break;
		case LOG_DEBUG: sevstring = "Debug"; break;
	}

	/* Only log to the database if we are above our minimum severity level */
	if (sev <= _minsev)
	{
		/* Construct a query to insert the log record */
		QString query;
		QTextOStream qos(&query);
		qos << "insert into logging (severity, profile,msgtime, message) values" 
				<< endl
				<< "('" << sevstring << "', '" << profile << "', NOW(),"
				<< "'" << escapeString(lm) << "');";
		q.exec(query);
	}

	/* If we are not detached, construct a string and send to the console.  use
	 * cerr for anything below LOG_WARNING, cout for everything else.  These
	 * messages are timestamped. */
	if (!srv->isdetached())
	{
		QString tm;
		QTextOStream os(&tm);

		os << "[" << sevstring << "] " <<
			QDateTime::currentDateTime().toString() << ": " << lm << endl;
		if (sev < LOG_WARNING)
		{
			cerr << tm;
		} else {
			cout << tm;
		}
	}

	/* Construct our in game string and emit the appropriate signal */
	{
		QString gm;
		QTextOStream os(&gm);

		os << "[" << sevstring << "]: " << lm << endl;
		switch (sev)
		{
			case LOG_FATAL: emit logfatalsent(gm); break;
			case LOG_SEVERE: emit logseveresent(gm); break;
			case LOG_CRITICAL: emit logcriticalsent(gm); break;
			case LOG_ERROR: emit logerrorsent(gm); break;
			case LOG_WARNING: emit logwarningsent(gm); break;
			case LOG_NOTICE: emit lognoticesent(gm); break;
			case LOG_INFO: emit loginfosent(gm); break;
			case LOG_DEBUG: emit logdebugsent(gm); break;
		}
	}
}

/** Escape special characters in strings
 * This is primarily to escape special strings before inserting a string into
 * the database.
 */
QString Logger::escapeString(QString str)
{
	QString out;
	out = str.replace(QRegExp("\\"), "\\\\");
	out = out.replace(QRegExp("'"), "\\'");
	return out;
}

	/** All commands go in this name space */
	namespace commands {
/** logging sub command tree */
koalamud::CommandTree *logsubcmdtree;

/** Log command class */
class Log : public Command /* {{{ */
{
	public:
		/** Pass through constructor */
		Log(Char *ch) : Command(ch) {}
		/** Run Log command
		 * This is actually a bunch of subcommands all together.  We parse out
		 * @a args to determine which subcommand we are executing.  To prevent
		 * building the same code multiple times, we build another command tree
		 * with all of the subcommands entered into it.  Then we simply need to
		 * extract the subcommand word from args and call back into the command
		 * tree with our new command and args. */
		virtual unsigned int run(QString args)
		{
			koalamud::Command *subcmd = NULL;

			/* Get subcommand word */
			QString word = args.section(' ', 0, 0);
			
			/* Lookup the subcommand on the logging subcommand tree */
			subcmd = logsubcmdtree->findandcreate(word, _ch, true);

			if (subcmd == NULL && word.length() > 1)
			{ /* Oops, invalid subcommand */
				QString str;
				QTextOStream os(&str);
				os << "Invalid subcommand.  See 'help logging' for more information."
				   << endl;
				_ch->sendtochar(str);
				return 1;
			} else if (subcmd == NULL) {
				/* No subcommand specified */
				QString str;
				QTextOStream os(&str);
				os << "You must specify a subcommand.  See 'help logging' for more information."
				   << endl;
				_ch->sendtochar(str);
				return 1;
			}

			/* Run the subcommand */
			return subcmd->run(args.section(' ', 1));
		}

		/** Restricted access command. */
		virtual bool isRestricted(void) const { return true;}

		/** Command Groups */
		virtual QStringList getCmdGroups(void) const
		{
			QStringList gl;
			gl << "Implementor" << "Coder" << "Builder" << "Immortal";
			return gl;
		}

		/** Get command name for individual granting */
		virtual QString getCmdName(void) const { return QString("logging"); }

	public:
	/** Set command class */
	class Set : public Command /* {{{ */
	{
		public:
			/** Pass through constructor */
			Set(Char *ch) : Command(ch) {}
			/** Run Set command
			 * Set minimum logging level to the value in args. */
			virtual unsigned int run(QString args)
			{
				QString newlevel;
				
				/* Go through each of the logging levels and check args for a match.
				 * When we find a match, set that level and return. */
				if (QString("fatal").startsWith(args.lower()))
				{
					Logger::instance()->setlevel(Logger::LOG_FATAL);
					newlevel = "Fatal";
				} else if (QString("severe").startsWith(args.lower()))
				{
					Logger::instance()->setlevel(Logger::LOG_SEVERE);
					newlevel = "Severe";
				} else if (QString("critical").startsWith(args.lower()))
				{
					Logger::instance()->setlevel(Logger::LOG_CRITICAL);
					newlevel = "Critical";
				} else if (QString("error").startsWith(args.lower()))
				{
					Logger::instance()->setlevel(Logger::LOG_ERROR);
					newlevel = "Error";
				} else if (QString("warning").startsWith(args.lower()))
				{
					Logger::instance()->setlevel(Logger::LOG_WARNING);
					newlevel = "Warning";
				} else if (QString("notice").startsWith(args.lower()))
				{
					Logger::instance()->setlevel(Logger::LOG_NOTICE);
					newlevel = "Notice";
				} else if (QString("info").startsWith(args.lower()))
				{
					Logger::instance()->setlevel(Logger::LOG_INFO);
					newlevel = "Info";
				} else if (QString("debug").startsWith(args.lower()))
				{
					Logger::instance()->setlevel(Logger::LOG_DEBUG);
					newlevel = "Debug";
				} else
				{
					QString out;
					QTextOStream os(&out);
					os << "Sorry, '" << args << "' is not a known logging severity"
						 << "level." << endl;

					_ch->sendtochar(out);
					return 1;
				}

				QString out;
				QTextOStream os(&out);

				os << "You have sucessfully resent minimum logging level to "
					 << newlevel << "." << endl;

				_ch->sendtochar(out);

				QString logmsg;
				QTextOStream ls(&logmsg);

				ls << _ch->getName() << " has reset minimum logging level to "
					 << newlevel;

				Logger::msg(logmsg);
				return 0;
			}

	}; /* }}} */
	/** Start command class */
	class Start : public Command /* {{{ */
	{
		public:
			/** Pass through constructor */
			Start(Char *ch) : Command(ch) {}
			/** Run Start command
			 * Start monitoring a specific logging level
			 * 
			 * @todo We should probably mark in the player class somehow that we are
			 * monitoring that log level so that we can display monitored log levels
			 */
			virtual unsigned int run(QString args)
			{
				QString newlevel;
				
				/* Go through each of the logging levels and check args for a match.
				 */
				if (QString("fatal").startsWith(args.lower()))
				{
					QObject::connect(Logger::instance(), SIGNAL(logfatalsent(QString)),
								_ch, SLOT(sendtochar(QString)));
					newlevel = "Fatal";
				} else if (QString("severe").startsWith(args.lower()))
				{
					QObject::connect(Logger::instance(), SIGNAL(logseveresent(QString)),
								_ch, SLOT(sendtochar(QString)));
					newlevel = "Severe";
				} else if (QString("critical").startsWith(args.lower()))
				{
					QObject::connect(Logger::instance(), SIGNAL(logcriticalsent(QString)),
								_ch, SLOT(sendtochar(QString)));
					newlevel = "Critical";
				} else if (QString("error").startsWith(args.lower()))
				{
					QObject::connect(Logger::instance(), SIGNAL(logerrorsent(QString)),
								_ch, SLOT(sendtochar(QString)));
					newlevel = "Error";
				} else if (QString("warning").startsWith(args.lower()))
				{
					QObject::connect(Logger::instance(), SIGNAL(logwarningsent(QString)),
								_ch, SLOT(sendtochar(QString)));
					newlevel = "Warning";
				} else if (QString("notice").startsWith(args.lower()))
				{
					QObject::connect(Logger::instance(), SIGNAL(lognoticesent(QString)),
								_ch, SLOT(sendtochar(QString)));
					newlevel = "Notice";
				} else if (QString("info").startsWith(args.lower()))
				{
					QObject::connect(Logger::instance(), SIGNAL(loginfosent(QString)),
								_ch, SLOT(sendtochar(QString)));
					newlevel = "Info";
				} else if (QString("debug").startsWith(args.lower()))
				{
					QObject::connect(Logger::instance(), SIGNAL(logdebugsent(QString)),
								_ch, SLOT(sendtochar(QString)));
					newlevel = "Debug";
				} else if (QString("all").startsWith(args.lower()))
				{
					QObject::connect(Logger::instance(), SIGNAL(logfatalsent(QString)),
								_ch, SLOT(sendtochar(QString)));
					QObject::connect(Logger::instance(), SIGNAL(logseveresent(QString)),
								_ch, SLOT(sendtochar(QString)));
					QObject::connect(Logger::instance(), SIGNAL(logcriticalsent(QString)),
								_ch, SLOT(sendtochar(QString)));
					QObject::connect(Logger::instance(), SIGNAL(logerrorsent(QString)),
								_ch, SLOT(sendtochar(QString)));
					QObject::connect(Logger::instance(), SIGNAL(logwarningsent(QString)),
								_ch, SLOT(sendtochar(QString)));
					QObject::connect(Logger::instance(), SIGNAL(lognoticesent(QString)),
								_ch, SLOT(sendtochar(QString)));
					QObject::connect(Logger::instance(), SIGNAL(loginfosent(QString)),
								_ch, SLOT(sendtochar(QString)));
					QObject::connect(Logger::instance(), SIGNAL(logdebugsent(QString)),
								_ch, SLOT(sendtochar(QString)));
					QString out;
					QTextOStream os(&out);

					os << "You have sucessfully started monitoring all log levels."
						 << endl;

					_ch->sendtochar(out);

					QString logmsg;
					QTextOStream ls(&logmsg);

					ls << _ch->getName() << " is now monitoring all log levels";

					Logger::msg(logmsg);
					return 0;
				} else
				{
					QString out;
					QTextOStream os(&out);
					os << "Sorry, '" << args << "' is not a known logging severity"
						 << "level." << endl;

					_ch->sendtochar(out);
					return 1;
				}

				QString out;
				QTextOStream os(&out);

				os << "You have sucessfully started monitoring log level "
					 << newlevel << "." << endl;

				_ch->sendtochar(out);

				QString logmsg;
				QTextOStream ls(&logmsg);

				ls << _ch->getName() << " is now monitoring log level "
					 << newlevel;

				Logger::msg(logmsg);
				return 0;
			}

	}; /* }}} */
	/** Stop command class */
	class Stop : public Command /* {{{ */
	{
		public:
			/** Pass through constructor */
			Stop(Char *ch) : Command(ch) {}
			/** Run Stop command
			 * Stop monitoring a specific log level */
			virtual unsigned int run(QString args)
			{
				QString newlevel;
				
				/* Go through each of the logging levels and check args for a match.
				 * When we find a match, set that level and return. */
				if (QString("fatal").startsWith(args.lower()))
				{
					Logger::instance()->disconnect(SIGNAL(logfatalsent(QString)), _ch);
					newlevel = "Fatal";
				} else if (QString("severe").startsWith(args.lower()))
				{
					Logger::instance()->disconnect(SIGNAL(logseveresent(QString)), _ch);
					newlevel = "Severe";
				} else if (QString("critical").startsWith(args.lower()))
				{
					Logger::instance()->disconnect(SIGNAL(logcriticalsent(QString)), _ch);
					newlevel = "Critical";
				} else if (QString("error").startsWith(args.lower()))
				{
					Logger::instance()->disconnect(SIGNAL(logerrorsent(QString)), _ch);
					newlevel = "Error";
				} else if (QString("warning").startsWith(args.lower()))
				{
					Logger::instance()->disconnect(SIGNAL(logwarningsent(QString)), _ch);
					newlevel = "Warning";
				} else if (QString("notice").startsWith(args.lower()))
				{
					Logger::instance()->disconnect(SIGNAL(lognoticesent(QString)), _ch);
					newlevel = "Notice";
				} else if (QString("info").startsWith(args.lower()))
				{
					Logger::instance()->disconnect(SIGNAL(loginfosent(QString)), _ch);
					newlevel = "Info";
				} else if (QString("debug").startsWith(args.lower()))
				{
					Logger::instance()->disconnect(SIGNAL(logdebugsent(QString)), _ch);
					newlevel = "Debug";
				} else if (QString("all").startsWith(args.lower()))
				{
					Logger::instance()->disconnect(SIGNAL(logfatalsent(QString)), _ch);
					Logger::instance()->disconnect(SIGNAL(logseveresent(QString)), _ch);
					Logger::instance()->disconnect(SIGNAL(logcriticalsent(QString)), _ch);
					Logger::instance()->disconnect(SIGNAL(logerrorsent(QString)), _ch);
					Logger::instance()->disconnect(SIGNAL(logwarningsent(QString)), _ch);
					Logger::instance()->disconnect(SIGNAL(lognoticesent(QString)), _ch);
					Logger::instance()->disconnect(SIGNAL(loginfosent(QString)), _ch);
					Logger::instance()->disconnect(SIGNAL(logdebugsent(QString)), _ch);
					QString out;
					QTextOStream os(&out);

					os << "You have sucessfully stopped monitoring all log levels."
						 << endl;

					_ch->sendtochar(out);

					QString logmsg;
					QTextOStream ls(&logmsg);

					ls << _ch->getName() << " has stopped monitoring all log levels";

					Logger::msg(logmsg);
					return 0;
				} else
				{
					QString out;
					QTextOStream os(&out);
					os << "Sorry, '" << args << "' is not a known logging severity"
						 << "level." << endl;

					_ch->sendtochar(out);
					return 1;
				}

				QString out;
				QTextOStream os(&out);

				os << "You have sucessfully stopped monitoring log level "
					 << newlevel << "." << endl;

				_ch->sendtochar(out);

				QString logmsg;
				QTextOStream ls(&logmsg);

				ls << _ch->getName() << " has stopped monitoring log level "
					 << newlevel;

				Logger::msg(logmsg);
				return 0;
			}

	}; /* }}} */
}; /* }}} Log Command class */

	}; /* End commands namespace */

using koalamud::commands::logsubcmdtree;
/** Command Factory for logging.cpp */
class Logging_CPP_CommandFactory : public CommandFactory
{
	public:
		/** Register our commands */
		Logging_CPP_CommandFactory(void)
			: CommandFactory()
		{
			maincmdtree->addcmd("logging", this, 1);

			/* Create subcommand tree */
			logsubcmdtree = new CommandTree();
			logsubcmdtree->addcmd("set", this, 2);
			logsubcmdtree->addcmd("stop", this, 3);
			logsubcmdtree->addcmd("start", this, 4);
		}

		/** Delete logging command tree */
		virtual ~Logging_CPP_CommandFactory(void)
		{
			delete koalamud::commands::logsubcmdtree;
		}

		/** Handle command object creations */
		virtual Command *create(unsigned int id, Char *ch)
		{
			switch (id)
			{
				case 1:
					return new koalamud::commands::Log(ch);
				case 2:
					return new koalamud::commands::Log::Set(ch);
				case 3:
					return new koalamud::commands::Log::Stop(ch);
				case 4:
					return new koalamud::commands::Log::Start(ch);
			}
			return NULL;
		}
};

/** Command factory for logging commands */
Logging_CPP_CommandFactory Logging_CPP_CommandFactoryInstance;

}; /** end koalamud namespace */
