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

#ifndef KOALA_PARSER_HXX
#define KOALA_PARSER_HXX "%A%"

/* predefine classes */
namespace koalamud {
	class Parser;
	class ParseDescriptor;
};

#include "char.hxx"

namespace koalamud {

/** Very basic parser base class to provide semantics to using classes */
class Parser
{
	public:
		/** Build a parser object */
		Parser(Char *ch = NULL, ParseDescriptor *desc = NULL)
			: _ch(ch), _desc(desc) {}
		/** Destroy a parser object */
		virtual ~Parser(void) {}

	public: /* operators */
		/** Operator new overload */
		void * operator new(size_t obj_size)
			{ return koalamud::PoolAllocator::alloc(obj_size); }
		/** Operator delete overload */
		void operator delete(void *ptr)
			{ koalamud::PoolAllocator::free(ptr); }

	public: /* pure virtual functions */
		/** Parse a line of input */
		virtual void parseLine(QString line) =0;

	protected:
		/** Pointer to attached character */
		Char *_ch;
		/** Pointer to attached descriptor */
		ParseDescriptor *_desc;
};

/** Parse player login information.
 * This handles the login sequence.  It will do the handling for creating new
 * characters, but it will switch over to character creation when a new
 * character is requested.
 * @note This will only handle player logins.  Mob controller login is
 * completely separate from this.
 */
class PlayerLoginParser : public Parser
{
	private:
		/** Current login state */
		typedef enum {
			STATE_GETNAME, /**< Getting player name */
			STATE_GETPASS, /**< Getting password */
			/** Confirm player name - they entered either new or a nonexistant name
			 */
			STATE_CONFNAME,
			/** Confirm player wants a new character */
			STATE_CONFNEW,
		} state_t;
		
	public:
		PlayerLoginParser(Char *ch = NULL, ParseDescriptor *desc = NULL);
		/** Destroy a parser object */
		virtual ~PlayerLoginParser(void) {}

	public: /* virtual functions */
		virtual void parseLine(QString line);

	protected:
		/** Player name */
		QString pname;
		/** Current parse state */
		state_t state;
};

/** Normal player parsing
 * This parser handles parsing input and searching out and running commands.
 * @todo Rewrite command executor task to work with new parser system and
 * the queue for commands
 *
 * @note This parser is useless without a character attached
 */
class PlayerParser : public Parser
{
	public:
		PlayerParser(Char *ch, ParseDescriptor *desc);
		/** Destroy a parser object */
		virtual ~PlayerParser(void) {}

	public: /* virtual functions */
		virtual void parseLine(QString line);
};

/** Player Creation Parser
 * This parser handles all of the new player creation parsing tasks, including
 * getting player name, race, initial role and class, and stats information.
 * This class also handles creating a basic database record for the new player
 * and spawning a character object for them before switching to the
 * PlayerParser.
 */
class PlayerCreationParser : public Parser
{
	public:
		PlayerCreationParser(ParseDescriptor *desc, QString name=NULL);
		/** Destroy a parser object */
		virtual ~PlayerCreationParser(void) {}

	public: /* virtual functions */
		virtual void parseLine(QString line);

	protected: /* Internal usage functions */
		void createDBRecord(void);
		/** Check to see if a name is allowed.
		 * @bug Temporary implementation */
		bool checkName(QString name)
			{ if (!name.isEmpty())return true; return false;}
		/** Check for a valid email address
		 * @bug Temporary implementation */
		bool checkEmail(QString email)
			{ if (!email.isEmpty())return true; return false;}

	protected: /* Various data gathered in creation process */
		/** Character first name */
		QString _fname;
		/** Character last name */
		QString _lname;
		/** Character password */
		QString _pass;
		/** Email address - Used for updates/password reminders, etc. */
		QString _email;

		/** Current process state values */
		typedef enum {
			STATE_GETNAME, /**< Get a character name */
			STATE_CONFNAME, /**< Confirm selected player name */
			STATE_GETLAST, /**< Get character last name */
			STATE_CONFLAST, /**< Confirm character last name */
			STATE_GETPASS, /**< Get password */
			STATE_CONFPASS, /**< Confirm password - they have to type it in again */
			STATE_GETEMAIL, /**< Get an email address from them */
			STATE_CONFEMAIL, /**< Confirm entered email */
		} state_t;

		/** Current parser state */
		state_t curstate;
};

}; /* end koalamud namespace */

#endif //  KOALA_PARSER_HXX
