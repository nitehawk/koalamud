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

#ifndef KOALA_LOGGING_HXX
#define KOALA_LOGGING_HXX "%A%"

#include <qobject.h>
#include <qstring.h>

namespace koalamud {

/** Database logger class
 * This class provides database logging functionality to the server.  It is a
 * singleton class and the static methods will automatically call into the
 * instantiated version.  The instance can be retrieved via instance().
 *
 * We will automatically pick up the execution profile from the MainServer
 * object while logging.  We will also copy all logging messages to the
 * terminal if we are not running in the background.
 */
class Logger : public QObject
{
	Q_OBJECT
	
	public:
		typedef enum {
			LOG_FATAL = 0, /**< Fatal error, shutting down */
			LOG_SEVERE, /**< Severe error, reboot likely required */
			LOG_CRITICAL, /**< Critical error, may be able to recover, but reboot suggested */
			LOG_ERROR, /**< Normal error, should be able to recover */
			LOG_WARNING, /**< Warning, check into this */
			LOG_NOTICE, /**< Something that should be seen, but isn't going to cause problems */
			LOG_INFO, /**< Informational message */
			LOG_DEBUG, /**< Debugging information */
		} log_lev; /**< Logging severity levels */

	protected:
		/** Lowest level we are logging to the database.
		 * All messages are logged to the console and all messages are sent via
		 * signals to subscribing players.
		 */
		log_lev _minsev;
		/** Server profile to log.  Should be set before logging stuff */
		QString profile;

		/** Force usage as a singleton.  Only instance() can instantiate us */
		Logger(void) : _minsev(LOG_INFO), profile("Default") {}

	public:
		static QString escapeString(QString str);
		void imsg(QString lm, log_lev sev = LOG_INFO);
		/** Log a message to the database */
		static void msg(QString lm, log_lev sev = LOG_INFO)
		{ instance()->imsg(lm, sev); }

		/** Return pointer to logger instance */
		static Logger *instance(void)
		{
			static Logger *inst = NULL;
			
			if (!inst)
				inst = new Logger;

			return inst;
		}

		/** Return the current minimum logged severity */
		log_lev level(void) { return _minsev; }
		/** Set the minimum severity for logged messages */
		void setlevel(log_lev sev) { _minsev = sev; }
		/** Set profile */
		static void setProfile(QString profile)
		{	instance()->profile = profile; }

/* These signals provide the link between the logger and interested listeners.
 * These signals all provide a preformatted string in the form of:
 * 		[severity] message
 *
 * This string is suitable for directly sending to players.  It is important
 * to note that the info severity will not get the same type of information
 * that an in game info channel would get, thus it is recommended that normal
 * players are not connected to any of the logging channels.  There will
 * likely be sensitive information passed on the logging channels, including
 * IP addresses and other related information.
 */
	signals:  
		/** Triggered for ever debug log message */
		void logdebugsent(QString message);
		/** Triggered for ever info log message */
		void loginfosent(QString message);
		/** Triggered for ever notice log message */
		void lognoticesent(QString message);
		/** Triggered for ever warning log message */
		void logwarningsent(QString message);
		/** Triggered for ever error log message */
		void logerrorsent(QString message);
		/** Triggered for ever critical log message */
		void logcriticalsent(QString message);
		/** Triggered for ever severe log message */
		void logseveresent(QString message);
		/** Triggered for ever fatal log message */
		void logfatalsent(QString message);
};

}; /* end koalamud namespace */

/* Extract Logger class into main namespace for convienence. */
using koalamud::Logger;

#endif //  KOALA_LOGGING_HXX
