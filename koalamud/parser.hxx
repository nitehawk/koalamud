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

}; /* end koalamud namespace */

#endif //  KOALA_PARSER_HXX
