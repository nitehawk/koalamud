/***************************************************************\
*                        KoalaMud Gen 2                         *
*    Copyright (c) 2002 First Step Internet Services, Inc.      *
*                     All Rights Reserved                       *
*        Distributed under the terms of the FSI License         *
* See file LICENSE in the root of this package for information  *
\***************************************************************/
/***************************************************************\
*	Module: Command
*	Author: Matthew Schlegel
* Description: Provides the command tree classes
* Classes:
\***************************************************************/

#ifndef KOALA_CMDTREE_HXX
#define KOALA_CMDTREE_HXX "%A%"

#include <zthread/FastRecursiveMutex.h>

#include "main.hxx"
#include "cmd.hxx"

namespace koalamud {

/** Defines a tree of commands
 * This class defines the command tree used in command lookups.  For
 * simplicity, we consolidate all of the command available type of checks to
 * the commands themselves.
 */
class CommandTree {
	public:
		/** An individual node in the command tree */
		class CommandTreeNode {
			protected:
				/** Pointer to the command factory */
				CommandFactory *_factory;
				/** Command Factory local ID */
				unsigned int _fid;
				/** Full name of this node */
				QString _name;
				/** Number of edges */
				static const unsigned int numedges = 27;
				/** Pointers to subordinate nodes - One for each letter + 1 for
				 * non-alpha */
				CommandTreeNode *edges[numedges];
				/** Pointer to parent */
				CommandTreeNode *parent;
				/** Depth is equivalent to the position in the name string that this
				 * node is using in the parents edges array.
				 * depth==2 means that for 'hello', parent->edgefor('e') == this
				 */
				unsigned short depth;
				
			public:
				/** Contructor takes command name, factory, and id
				 * @param name Full command name
				 * @param fact Pointer to factory
				 * @param id Command ID
				 */
				CommandTreeNode(QString name, CommandFactory *fact, unsigned int id)
					: _factory(fact), _fid(id), _name(name), parent(NULL), depth(0)
					{ for (unsigned int i = 0; i < numedges; i++) { edges[i] = NULL; } }

				unsigned short edgenum(char letter);

				/** Instantiate the class
				 * @param ch Pointer to character
				 */
				Command *createcommand(Char *ch)
				{ if (_factory != NULL) return _factory->create(_fid, ch);
					else return NULL; }

			public:
				/** Operator new overload */
				void * operator new(size_t obj_size)
					{ return koalamud::PoolAllocator::alloc(obj_size); }
				/** Operator delete overload */
				void operator delete(void *ptr)
					{ koalamud::PoolAllocator::free(ptr); }

				/** Make sure that CommandTree can manipulate our data (for traversal
				 * and adding nodes */
				friend class CommandTree;
		}; /* End class CommandTreeNode */

	public:
		CommandTree(void);
		~CommandTree(void);
		/** Operator new overload */
		void * operator new(size_t obj_size)
			{ return koalamud::PoolAllocator::alloc(obj_size); }
		/** Operator delete overload */
		void operator delete(void *ptr)
			{ koalamud::PoolAllocator::free(ptr); }

		bool addcmd(QString name, CommandFactory *fact, unsigned int id);

		CommandTreeNode *find_full(QString cmd);
		CommandTreeNode *find_abbrev(QString cmd);

		/** Find a command and create a command object
		 * Search the command tree and create a command object based on what we
		 * find.
		 * @param cmd Command to find
		 * @param ch Character pointer to pass on to command
		 * @param abbrev true if we want to match abbreviations
		 * @return pointer to new command object or null if it could not be found.
		 */
		Command *findandcreate(QString cmd, Char *ch, bool abbrev = false)
		{
			CommandTreeNode *cmdptr = NULL;
			
			if (abbrev)
				cmdptr = find_abbrev(cmd);
			else
				cmdptr = find_full(cmd);

			if (cmdptr)
				return cmdptr->createcommand(ch);

			return NULL;
		}

	protected:
		/** Pointer to the root of the CommandTree */
		CommandTreeNode *rootnode;
		/** Lock for the tree to prevent corruption */
		ZThread::FastRecursiveMutex treelock;

		void destroybranch(CommandTreeNode *branch);
};

/** Singleton wrapper for the main command tree
 * This class simply provides a wrapper around the main command tree that acts
 * as a specialized smart pointer singleton
 */
class MainCommandTree
{
	public:
		/** Detroy main command tree */
		~MainCommandTree() { delete maincmdtree; }
		/** Return a pointer to the main command tree */
		CommandTree * operator->()
		{
			static bool initialized = false;
			if (!initialized)
			{
				maincmdtree = new CommandTree;
				initialized = true;
			}
			return maincmdtree;
		}

	protected:
		/** Pointer to our command tree */
		CommandTree *maincmdtree;
};

}; /* KoalaMud namespace */


#ifdef KOALA_CMDTREE_CXX
koalamud::MainCommandTree  maincmdtree;
#else
extern koalamud::MainCommandTree  maincmdtree;
#endif

#endif // KOALA_CMDTREE_HXX
