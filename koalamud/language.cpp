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

#define KOALA_LANGUAGE_CXX "%A%"

#include <stdlib.h>

#include "logging.hxx"
#include "language.hxx"
#include "cmd.hxx"
#include "cmdtree.hxx"

namespace koalamud
{

/** Generate a charmap for this language
 * This is called at regular intervals to make sure that the mapping doesn't
 * stay constant, but that it doesn't normally change from instant to instant
 */
void Language::genCharMap(void)
{
	/* We want at least 4 characters to provide some variety */
	if (_charset.length() <= 3)
	{
		for(int i=0; i < charmaplen; i++)
		{
			charmap[i] = 'z' - i;
		}
		return;
	}

	/* Fill in charmap with random characters from _charset  */
	unsigned int charsetlen = _charset.length();
	for (int i=0; i < charmaplen; i++)
	{
		int randpos = (int)(charsetlen*(random()/(RAND_MAX+1.0)));
		charmap[i] = _charset[randpos].latin1();
	}
}

/** Morph an input string to a given know percentage */
QString Language::morphString(QString in, int know)
{
	QString out = in;

	/* Short circuit fully known language */
	if (know >= 100)
		return out;

	int msglen = in.length();

	/* Randomly change up to ((100-know)/100 * strlen(in))-rand(20) chars
	 * This *should* result in a mostly readable string most of the time, and
	 * should allow shop usage in a language with at least 75% know.  */
	if (know >= 75)
	{
		int replace = (int)((((100-know)/100)*msglen)*(random()/(RAND_MAX+1.0)));
		for (int i=0; i < replace; i++)
		{
			int randpos = (int)(msglen*(random()/(RAND_MAX+1.0)));
			out[randpos] = mapChar(in[randpos]);
		}
	} else if (know <= 5) {
		/* If the language know is less then or equal to 5%, every char gets
		 * morphed */
		for (int i=0; i < msglen; i++)
		{
			out[i] = mapChar(in[i]);
		}
	} else {
		/* We go through each character and generate a random number between 0 and
		 * 100.  If the random number is greater then know, we morph it. */
		for (int i=0; i < msglen; i++)
		{
			if ((100*(random()/(RAND_MAX+1.0)))> know)
			{
				out[i] = mapChar(in[i]);
			}
		}
	}
	return out;
}

/** Load languages from the database and create language objects */
void Language::loadLanguages(void)
{
	QString query = "select langid, name, parentid, charset, difficulty, shortname from languages;";
	QSqlQuery q(query);
	while (q.next())
	{
		new Language(q.value(0).toString(), q.value(1).toString(),
								 q.value(2).toString(), q.value(3).toString(),
								 q.value(4).toUInt(), q.value(5).toString());
	}
}

/** Start editing a language in OLC */
LanguageOLC::LanguageOLC(Char *ch, ParseDescriptor *pd, Parser *oldParser,
                  QString id)
	: olc(ch, pd, oldParser), langid(id) 
{
	addField(QString("Language ID"), FIELD_STRING, &langid, false, 5);
	addField(QString("Language Name"), FIELD_STRING, &name, true, 50);
	addField(QString("Parent Language ID"), FIELD_STRING, &parentlang, true, 5);
	addField(QString("Language CharSet"), FIELD_STRING, &charset, true, 50);
	addField(QString("Notes"), FIELD_STRING, &notes, true, 255);
	addField(QString("Difficulty"), FIELD_INTEGER, &difficulty, true, 100,0);
	addField(QString("ShortName"), FIELD_STRING, &shortname, true, 20);

	/* langid will already be checked for validity, so don't worry about it
	 * here. */
	if (load())
	{
		sendMenu();
		return;
	}
	name = "New Language";
	charset = "abcdefghijklmnopqrstuvwxyz";
	notes = "No language notes";
	difficulty = 50;
	shortname = "language";
	sendMenu();
}

/** Save Language to database and reload in game version */
void LanguageOLC::save(void)
{
	QString query;
	QTextOStream qos(&query);
	QSqlQuery q;
	
	qos << "replace into languages " << endl
			<< "(langid, name, parentid, charset, notes, difficulty,shortname)"
			<< endl << "values" << endl
			<< "('" << langid << "', '"
			<< Logger::escapeString(name) << "', '"
			<< parentlang << "', '"
			<< Logger::escapeString(charset) << "', '"
			<< Logger::escapeString(notes) << "', " << difficulty
			<< ", '" << shortname << "');";
	q.exec(query);

	/* Reload language */
	delete Language::getLanguage(langid);
	new Language(langid, name, parentlang, charset, difficulty, shortname);
}

/** Load language from database for editing */
bool LanguageOLC::load(void)
{
	QString query;
	QTextOStream qos(&query);
	QSqlQuery q;

	qos << "select langid, name, parentid, charset, notes, difficulty," << endl
			<< "shortname" << endl
			<< "from languages where langid = '" << langid << "';";
	if (q.exec(query) && q.numRowsAffected())
	{
		q.next();
		langid = q.value(0).toString();
		name = q.value(1).toString();
		parentlang = q.value(2).toString();
		charset = q.value(3).toString();
		notes = q.value(4).toString();
		difficulty = q.value(5).toUInt();
		shortname = q.value(6).toString();
		return true;
	}
	return false;
}

namespace commands {

/** Language Edit command class */
class LangEdit : public Command
{
	public:
		/** Pass through constructor */
		LangEdit(Char *ch) : Command(ch) {}
		/** Run Lang edit command */
		virtual unsigned int run(QString args)
		{
			QString langid = args.left(5);

			for (int i = 0; i < 5; i++)
			{
				if (!langid[i].isLetter())
				{
					QString out;
					QTextOStream os(&out);
					os << "|RLanguage IDs must be completely alpha characters, no numbers or symbols.|x" << endl;
					_ch->sendtochar(out);
					return 1;
				}
			}
			
			Parser *_oldparser = _ch->getDesc()->parser();
			_ch->getDesc()->setParser(
					new LanguageOLC(_ch, _ch->getDesc(), _oldparser, langid), false);
			return 0;
		}

		/** Restricted access command. */
		virtual bool isRestricted(void) const { return true;}

		/** Command Groups */
		virtual QStringList getCmdGroups(void) const
		{
			QStringList gl;
			gl << "Implementor" << "Coder" << "Builder";
			return gl;
		}

		/** Get command name for individual granting */
		virtual QString getCmdName(void) const { return QString("langedit"); }
};

/** Language List command class */
class LangList : public Command
{
	public:
		/** Pass through constructor */
		LangList(Char *ch) : Command(ch) {}
		/** Run Lang List command */
		virtual unsigned int run(QString args)
		{
			QString out;
			QTextOStream os(&out);
			
			os << "|BLoaded languages|x" << endl
				 << "|MLanguageID    Short Name         Language Name|x" << endl;

			QDictIterator<Language> lang(languageMap);
			for( ; lang.current(); ++lang )
			{
				os << "  " << (*lang)->getID() << "      "
					 << (*lang)->getShort() << "      "
					 << (*lang)->getName() << endl;
			}
			
			os << endl;

			_ch->sendtochar(out);
			return 0;
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
		virtual QString getCmdName(void) const { return QString("langlist"); }
};

}; /* end commands namespace */

/** Command Factory for help.cpp */
class Language_CPP_CommandFactory : public CommandFactory
{
	public:
		/** Register our commands */
		Language_CPP_CommandFactory(void)
			: CommandFactory()
		{
			maincmdtree->addcmd("langedit", this, 1);
			maincmdtree->addcmd("langlist", this, 2);
		}

		/** Handle command object creations */
		virtual Command *create(unsigned int id, Char *ch)
		{
			switch (id)
			{
				case 1:
					return new koalamud::commands::LangEdit(ch);
				case 2:
					return new koalamud::commands::LangList(ch);
			}
			return NULL;
		}
};

/** Command factory for help.cpp module.  */
Language_CPP_CommandFactory Language_CPP_CommandFactoryInstance;

}; /* end koalamud namespace */
