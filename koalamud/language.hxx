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

#ifndef KOALA_LANGUAGE_HXX
#define KOALA_LANGUAGE_HXX "%A%"

#include <qmap.h>

#include "memory.hxx"
#include "olc.hxx"

namespace koalamud
{

class Language;

#ifdef KOALA_LANGUAGE_CXX
/** Map of language ids to languages */
QMap<QString,QString> langidmap;
/** Map of language names to ids */
QMap<QString,QString> langnamemap;
/** Map of short language names to ids */
QMap<QString,QString> snamemap;
/** Map language IDs to language objects */
QDict<Language> languageMap;
#else
/** Map of language ids to languages */
extern QMap<QString,QString> langidmap;
/** Map of language names to ids */
extern QMap<QString,QString> langnamemap;
/** Map of short language names to ids */
extern QMap<QString,QString> snamemap;
/** Map language IDs to language objects */
extern QDict<Language> languageMap;
#endif

/** Language Class
 * This class provides the interface for the language morphing code.  It does
 * not handle the actual marker extraction, however.  Marker extraction is
 * done in the Char class, specifically sendtochar.
 */
class Language
{
	public:
		/** Mapping in charmap */
		static const int charmaplen = 26;

	public:
		/** Construct a language object */
		Language(QString langid, QString name, QString parentlang,
							QString charset, unsigned int difficulty, QString sname)
			: _id(langid), _name(name), _parent(parentlang), _shortname(sname),
				_charset(charset), _difficulty(difficulty)
			{ genCharMap(); languageMap.insert(langid, this);
				langidmap[langid] = name;  langnamemap[name] = langid;
				snamemap[sname] = langid; }
		/** Destroy a language object (remove ourself from the maps) */
		~Language(void)
			{ langidmap.erase(_id); langnamemap.erase(_name);
				languageMap.remove(_id); }

		void genCharMap(void);
		QString morphString(QString in, int know);

		/** Get morph character for a specified character */
		QChar mapChar(QChar in)
			{ if ((in < 'a' || in > 'z') && (in < 'A' || in > 'Z'))
					return in;
				if (in >= 'a' && in <= 'z') return QChar(charmap[(in.latin1()-'a')]);
				return QChar(charmap[(in.latin1() - 'A')]).upper(); }

	public: /* statics */
		/** Get a pointer to the Language object for a specific language */
		static Language *getLanguage(QString langid)
			{ return languageMap[langid]; }
		/** Map language name to language ID */
		static QString getLangID(QString name)
			{ return langnamemap[name]; }
		/** Map language ID to language name */
		static QString getLangName(QString ID)
			{ return langidmap[ID]; }
		/** Map short name to language ID */
		static QString getLangIDfromShort(QString shortname)
			{ return snamemap[shortname]; }
		static void loadLanguages(void);
			
	public:
		/** Operator new overload */
		void * operator new(size_t obj_size)
			{ return koalamud::PoolAllocator::alloc(obj_size); }
		/** Operator delete overload */
		void operator delete(void *ptr)
			{ koalamud::PoolAllocator::free(ptr); }

	public:  /* Property gets */
		/** Get language ID */
		QString getID(void) const { return _id; }
		/** Get language name */
		QString getName(void) const { return _name; }
		/** Get language difficulty */
		unsigned int getDifficulty(void) const { return _difficulty; }
		/** Get language parent */
		QString getParent(void) const { return _parent; }
		/** Get language shortname */
		QString getShort(void) const { return _shortname; }

	protected:
		QString _id;  /**< langid - used in markers */
		QString _name;	/**< Language name */
		QString _parent; /**< Parent language ID */
		QString _shortname; /**< Short Language Name */
		/** Characters to use for this language (all lowercase)
		 * Characters can be listed more then once to increase weighting of a
		 * specific character */
		QString _charset;
		/** Difficulty of learning language */
		unsigned int _difficulty;

		/** Map chars into language specific set */
		char charmap[charmaplen];
};

/** Language editor class
 * This class allows us to add or modify languages in a running game.  In the
 * long run, this will probably be used mainly to adjust the charsets to
 * change the flavoring of languages slightly.
 */
class LanguageOLC : public olc
{
	public:
		LanguageOLC(Char *ch, ParseDescriptor *pd, Parser *oldParser,
								QString id);

	public:
		virtual void save(void);
		virtual bool load(void);

	protected:
		/** Language ID */
		QString langid;
		/** Language Name */
		QString name;
		/** Parent language ID */
		QString parentlang;
		/** Charset that language uses */
		QString charset;
		/** Language notes, short note about flavoring, etc. */
		QString notes;
		/** Language Difficulty */
		long difficulty;
		/** Shortname - used in changing languages */
		QString shortname;
};
	
}; /* end koalamud namespace */

#endif //  KOALA_LANGUAGE_HXX
