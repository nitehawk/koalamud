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

#define KOALA_HELP_CXX "%A%"

#include "help.hxx"
#include "logging.hxx"

namespace koalamud {
	namespace commands {

/** Help command class
 * This class will display help on a specified topid or list topics with
 * matching entries if there is more than one entry with the specified
 * keyword.  Entries can be specified either by keyword or by ID number.  This
 * will also allow searching for entries with the form 'help search <search
 * text>'
 */
class Help : public Command
{
	public:
		/** Pass through constructor */
		Help(Char *ch) : Command(ch) {}
		/** Run help command */
		virtual unsigned int run(QString args)
		{
			QString str;
			QTextOStream os(&str);
			QString topic = args.section(' ', 0, 0);
			QString searchargs = args.section(' ', 1);
			int topnum = topic.toInt();
			QSqlQuery q;

			/* If we didn't get any arguments, set topnum to display the first help
			 * topic in the database which should be an overview page
			 * @todo Don't display help pages that _ch does not have access to. */
			if (topic.isEmpty())
			{
				topnum = 1;
			}

			/* If we have a topic number, we just grab that entry and display it or
			 * return an error */
			if (topnum)
			{
				QString query;
				QTextOStream qos(&query);

				qos << "select title, body from helptext where helpid = " << topnum
						<< ";";
				if (q.exec(query))
				{
					if (q.numRowsAffected())
					{
						q.next();
						os << "|GHelp:|B " << q.value(0).toString() << "|x" <<endl<< endl 
							 << q.value(1).toString() << endl;
						_ch->sendtochar(str);
						return 0;
					}
					os << "That help topic does not exist." << endl;
					_ch->sendtochar(str);
					return 0;
				}
			}

			/* if our help topic is 'search' check for additional data in args,
			 * otherwise simply look for topic 'search' as usual. */
			if (topic.lower() == "search")
			{
				if (!searchargs.isEmpty()) 
				{
					/* Do a full text search and return the records in order of
					 * relevance */
					QString query;
					QTextOStream qos(&query);
					qos << "Select helpid, title from helptext " << endl;
					qos << "where match(title,keywords,body) " << endl;
					qos << "against ('" << searchargs << "');";
					if (q.exec(query))
					{
						if (unsigned int total = q.numRowsAffected())
						{
							os << "Help topic search results matching: " << endl
								 << "'" << searchargs << "'" << endl;
							os << "|mTopic Number|x     |bTitle|x" << endl;
							unsigned int count = 0;
							while (q.next() && count < 18)
							{
								count++;
								os.width(12);
								os << q.value(0).toString();
								os.width(0);
							os << "  " << q.value(1).toString() << endl;
							}

							os << endl << "Displayed first |r" << count <<"|x results of |r"
								 << total << "|x results total." << endl;
							_ch->sendtochar(str);
						} else {
							os << "No help topics matched your query." << endl;
							_ch->sendtochar(str);
						}
						return 0;
					}
				}
			}

			/* Do a normal help lookup.  If we get 1 result, display it.  If we get
			 * more than one result, display a listing similar to our search
			 * listing.  Otherwize say we couldn't find any matches */
			{
				QString query;
				QTextOStream qos(&query);
				qos << "Select helpid, title, body from helptext" << endl;
				qos << "where keywords like '%" << args << "%';";
				if (q.exec(query))
				{
					unsigned int total;
					if ((total = q.numRowsAffected()) == 1)
					{
						q.next();
						os << "|GHelp:|B " << q.value(1).toString()<< "|x" << endl << endl
						   << q.value(2).toString() << endl;
						_ch->sendtochar(str);
						return 0;
					} else if (total > 1) {
						os << "Help topic search results matching: " << endl
							 << "'" << searchargs << "'" << endl;
						os << "|mTopic Number      |bTitle|x" << endl;
						unsigned int count = 0;
						while (q.next() && count < 18)
						{
							count++;
							os.width(12);
							os << q.value(0).toString();
							os.width(0);
							os << "  " << q.value(1).toString() << endl;
						}

						os << endl << "Displayed first |r" << count << "|x results of |r"
							 << total << "|x results total." << endl;
						_ch->sendtochar(str);
						return 0;
					} else {
						os << "That help topic was not found." << endl
							 << "Try '|Yhelp search " << args << "|x'" << endl;
						_ch->sendtochar(str);
						return 0;
					}
				}
			}
			return 1;
		}
};

/** Help Edit command class
 */
class HelpEdit : public Command
{
	public:
		/** Pass through constructor */
		HelpEdit(Char *ch) : Command(ch) {}
		/** Run help edit command */
		virtual unsigned int run(QString args)
		{
			QString str;
			QTextOStream os(&str);
			QString topic = args.section(' ', 0, 0);
			int topnum = topic.toInt();
			QSqlQuery q;

			/* If we didn't get any arguments, set topnum to display the first help
			 * topic in the database which should be an overview page
			 * @todo Don't display help pages that _ch does not have access to. */
			if (topic.isEmpty())
			{
				os << "You must specify a help entry to edit" << endl;
				_ch->sendtochar(str);
				return 1;
			}

			/* if our help topic is 'search' check for additional data in args,
			 * otherwise simply look for topic 'search' as usual. */
			if (topic.lower() == "new")
			{
				topnum = 0;
			}

			/* Do a normal help lookup.  If we get 1 result, display it.  If we get
			 * more than one result, display a listing similar to our search
			 * listing.  Otherwize say we couldn't find any matches */
			{
				QString query;
				QTextOStream qos(&query);
				qos << "Select helpid, title, body from helptext" << endl;
				qos << "where keywords like '%" << args << "%';";
				if (q.exec(query))
				{
					unsigned int total;
					if ((total = q.numRowsAffected()) == 1)
					{
						q.next();
						topnum = q.value(0).toInt();
					} else if (total > 1) {
						topnum = -1;
						os << "Help topic search results matching: " << endl
							 << "'" << args << "'" << endl;
						os << "Topic Number      Title" << endl;
						unsigned int count = 0;
						while (q.next() && count < 18)
						{
							count++;
							os.width(12);
							os << q.value(0).toString();
							os.width(0);
							os << "  " << q.value(1).toString() << endl;
						}

						os << endl << "Displayed first " << count << " results of "
							 << total << " results total." << endl;
						_ch->sendtochar(str);
						return 0;
					}
				}
			}

			if (topnum >=0)
			{
				Parser *_oldparser = _ch->getDesc()->parser();
				_ch->getDesc()->setParser(
							new HelpOLC(_ch, _ch->getDesc(), _oldparser, topnum), false);
				return 0;
			}

			return 1;
		}

		/** Restricted access command. */
		virtual bool isRestricted(void) const { return true;}

		/** Command Groups */
		virtual QStringList getCmdGroups(void) const
		{
			QStringList gl;
			gl << "Implementor" << "Coder" << "Builder" << "HelpEditor";
			return gl;
		}

		/** Get command name for individual granting */
		virtual QString getCmdName(void) const { return QString("helpedit"); }
};

	}; /* end commands namespace */

/** Command Factory for help.cpp */
class Help_CPP_CommandFactory : public CommandFactory
{
	public:
		/** Register our commands */
		Help_CPP_CommandFactory(void)
			: CommandFactory()
		{
			maincmdtree->addcmd("help", this, 1);
			maincmdtree->addcmd("helpedit", this, 2);
		}

		/** Handle command object creations */
		virtual Command *create(unsigned int id, Char *ch)
		{
			switch (id)
			{
				case 1:
					return new koalamud::commands::Help(ch);
				case 2:
					return new koalamud::commands::HelpEdit(ch);
			}
			return NULL;
		}
};

/** Command factory for help.cpp module.  */
Help_CPP_CommandFactory Help_CPP_CommandFactoryInstance;

/** Start editing a help entry */
HelpOLC::HelpOLC(Char *ch, ParseDescriptor *pd, Parser *oldParser,
						long entry = 0)
	: olc(ch, pd, oldParser), _entry(entry)
{
	/* Add fields */
	addField(QString("Help ID"), FIELD_INTEGER, &_entry, false);
	addField(QString("Title"), FIELD_STRING, &_topic, true, 80);
	addField(QString("Keywords"), FIELD_STRING, &_keywords, true, 255);
	addField(QString("Body"), FIELD_MULTILINE, &_body, true);

	/* Load entry */
	if (entry > 0)
	{
		if (load())
		{
			sendMenu();
			return;
		}
		_entry = 0;
	}
	_topic = "New Topic";
	_keywords = "New Keywords";
	_body = "New Help Entry";
	sendMenu();
}

/** Load help entry from database for editing. */
bool HelpOLC::load(void)
{
	QString query;
	QTextOStream qos(&query);
	QSqlQuery q;

	qos << "select helpid, title, keywords, body from helptext " << endl
			<< "where helpid = " << _entry << ";";
	if (q.exec(query) && q.numRowsAffected())
	{
		q.next();
		_entry = q.value(0).toUInt();
		_topic = q.value(1).toString();
		_keywords = q.value(2).toString();
		_body = q.value(3).toString();
		return true;
	}

	return false;
}

/** Save help entry to database */
void HelpOLC::save(void)
{
	QString query;
	QTextOStream qos(&query);
	QSqlQuery q;

	if (_entry == 0)
	{
		qos << "insert into helptext (title, keywords, body) values " << endl
				<< "('" << _topic << "', '" << _keywords << "', '" << _body << "');";
	}
	else
	{
		qos << "update helptext " << endl
				<< "set title='" << _topic << "', " << endl
				<< "keywords='" << _keywords << "', " << endl
				<< "body='" << Logger::escapeString(_body) << "'" << endl
				<< "where helpid = " << _entry << ";";
	}
	if (q.exec(query))
	{
		if (_entry == 0)
		{
			/* Doesn't look like we can get the last insert ID, so we'll query for a
			 * specific match to our title and keywords */
			QString q2;
			QTextOStream os2(&q2);
			os2 << "select helpid from helptext where " << endl
					<< "title = '" << _topic << "' and keywords = '" << _keywords
					<< "';";
			if (q.exec(q2) && q.next())
			{
				_entry = q.value(0).toUInt();
			}
		}
	} else {
		QString out;
		QTextOStream os(&out);
		os << "Failed save query in help editor: " << query;
		Logger::msg(out, Logger::LOG_ERROR);
	}
}

}; /* end koalamud namespace */
