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
#include "exception.hxx"

/* Predefine types */
namespace koalamud {
	class Command;
};

#include "char.hxx"

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
		Char *_ch;
		/** True if we are overriding permissions - may not be needed */
		bool _overrideperms;

	public:
		/** Normal Constructor 
		 * @param ch Pointer to player executing command
		 * @param overrideperms Override command permissions (used in granting a
		 * command to make sure it is a valid command before updating the
		 * database)
		 */
		Command(Char *ch, bool overrideperms=false)
				: _ch(ch), _overrideperms(overrideperms) {}

		virtual unsigned int runCmd(QString args)
				throw (koalamud::exceptions::cmdpermdenied);
		/** Virtual destructor to make sure we delete things properly */
		virtual ~Command(void) {}

		/** Run the command 
		 * @todo This will need to accept some parameters to attach the command to
		 * parameters and other information needed to actually run the command */
		virtual unsigned int run(QString args) = 0;

		/** Is this a restricted command
		 * Default is unrestricted.  Override in command trees that are restricted
		 * or in single commands to return true. */
		virtual bool isRestricted(void) const { return false; }

		/** Get a list of command groups that this command belongs to.
		 * Return a QStringList of command groups.  These must match the gnames in
		 * the database exactly. */
		virtual QStringList getCmdGroups(void) const { QStringList ql; return ql; }

		/** Return the name of this command.
		 * This is used in looking up specific grant/deny permissions
		 * Reimplement this function if individual grant/deny is desired on this
		 * command */
		virtual QString getCmdName(void) const { return ""; }

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
		 */
		virtual Command *create(unsigned int id , Char *ch) = 0;
};

}; /* Koalamud namespace */

#endif // KOALA_CMD_HXX
