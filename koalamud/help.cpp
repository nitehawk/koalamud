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
						os << "Help: " << q.value(0).toString() << endl << endl 
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
						os << "Help: " << q.value(1).toString() << endl << endl
						   << q.value(2).toString() << endl;
						_ch->sendtochar(str);
						return 0;
					} else if (total > 1) {
						os << "Help topic search results matching: " << endl
							 << "'" << searchargs << "'" << endl;
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
					} else {
						os << "That help topic was not found." << endl
							 << "Try 'help search " << args << "'" << endl;
						_ch->sendtochar(str);
						return 0;
					}
				}
			}
			return 1;
		}
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
		}

		/** Handle command object creations */
		virtual Command *create(unsigned int id, Char *ch)
		{
			switch (id)
			{
				case 1:
					return new koalamud::commands::Help(ch);
			}
			return NULL;
		}
};

/** Command factory for help.cpp module.  */
Help_CPP_CommandFactory Help_CPP_CommandFactoryInstance;
}; /* end koalamud namespace */
