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

#ifndef KOALA_CMD_HXX
#define KOALA_CMD_HXX "%A%"

#include <qdict.h>
#include <qstring.h>

#include "main.hxx"
#include "playerchar.hxx"

/* New Command stuff */
namespace koalamud {

/** Command base class
 *
 * This is the abstract base class for all commands in the system. It provides
 * the basic functionality for the system, including new/delete overloading,
 * but leaves 'run()' as a pure virtual function
 * New commands must implement the constructor and the run function */
class Command
{
	protected:
		/** Pointer to player executing this command */
		K_PlayerChar *_ch;

	public:
		/** Normal Constructor 
		 * @param ch Pointer to player executing command
		 */
		Command(K_PlayerChar *ch) : _ch(ch) {}
		/** Virtual destructor to make sure we delete things properly */
		virtual ~Command(void) {}

		/** Run the command 
		 * @todo This will need to accept some parameters to attach the command to
		 * parameters and other information needed to actually run the command */
		virtual unsigned int run(QString cmd, QString args) = 0;

		/** Operator new overload */
		void * operator new(size_t obj_size)
			{ return koalamud::PoolAllocator::alloc(obj_size); }
		/** Operator delete overload */
		void operator delete(void *ptr)
			{ koalamud::PoolAllocator::free(ptr); }
};

/** Command class factory base class
 *
 * This class provides the base class for all class factories in the command
 * system.  It is an abstract base class that cannot be instantiated.
 *
 * @author Matthew Schlegel <nitehawk@koalamud.org>
 */
class CommandFactory
{
	public:
		/** Null constructor for base class
		 * For the full implementations of this class, the constructor will add
		 * all the commands that it creates for into the command tree.
		 */
		CommandFactory(void) {}
		/** Null destructor for base class
		 * Normally this removes any managed commands from the command tree
		 */
		virtual ~CommandFactory(void) {}

		/** Create a command object
		 * The command tree includes a pointer to the factory and an unsigned int
		 * identifier for each command.  Those values are used to call this
		 * function which will return a pointer to a new command object.
		 * @param id used as an identifier specific to a specific command factory
		 * @param ch Pointer to the character executing this command.
		 * @return Pointer to new Command object
		 *
		 * @todo PlayerChar class needs to be rebuilt in the koalamud namespace
		 * before we make use of it for the real version of this function.
		 */
		virtual Command *create(unsigned int id , K_PlayerChar *ch) = 0;
		/*virtual Command *create(unsigned int id , PlayerChar *ch) = 0; */
};

}; /* Koalamud namespace */

#define KOALACMD(cmdn) void cmdn(K_PlayerChar *ch, QString cmd, QString args)

#endif // KOALA_CMD_HXX
