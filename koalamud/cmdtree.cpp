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

#define KOALA_CMDTREE_CXX "%A%"

#include <ctype.h>
#include <qregexp.h>

#include "cmdtree.hxx"

namespace koalamud {

/*******
 * Stuff for the CommandTreeNodes
 */

/** Return a pointer to the command tree node for a given edge
 * @param let Letter to return edge for.  This is lowercased and normalized as
 * needed automatically.
 * @return index into edges for the given char
 */
unsigned short CommandTree::CommandTreeNode::edgenum(char let)
{
	unsigned char lowerlet = tolower(let);
	if (lowerlet < 'a' || lowerlet > 'z')
		return numedges - 1;
	else
		return lowerlet - 'a';
}

/*******
 * Stuff for the CommandTree
 */

/** Build a CommandTree object
 * This constructor builds an empty CommandTree by creating a new root node
 * and setting up all of our tracking variables
 */
CommandTree::CommandTree(void)
{
	rootnode = new CommandTreeNode("", NULL, 0);
}

/** Destroy CommandTree
 * Run a depth first traversal of the entire CommandTree, deleting the nodes
 * as we come upwards in the traversal
 */
CommandTree::~CommandTree(void)
{
	destroybranch(rootnode);
	delete rootnode;
}

/** Destroy a branch of the CommandTree
 * Recursively destroy a branch of a CommandTree
 * @param branch Branch to destroy
 */
void CommandTree::destroybranch(CommandTreeNode *branch)
{
	for (unsigned int i=0; i<branch->numedges; i++)
	{
		if (branch->edges[i])
		{
			destroybranch(branch->edges[i]);
			delete branch->edges[i];
		}
	}
}

QString CommandTree::displayBranch(void)
{
	QString el;
	QTextOStream os(&el);
	os << endl;
	return displayBranch(rootnode).join(el);
}

/** Build a QString of a branch of the command tree
 * If branch is null, start at the root.
 * @note Recursive function */
QStringList CommandTree::displayBranch(CommandTreeNode *branch,
										QString indent = "")
{
	QStringList out;
	if (branch != rootnode)
	{
		QString cur;
		QTextOStream os(&cur);
		os << indent << branch->depth << "  " <<  branch->_name;
		out << cur;
	}
	indent += "  ";

	for (unsigned int i = 0; i < branch->numedges; i++)
	{
		if (branch->edges[i])
		{
			out += displayBranch(branch->edges[i], indent);
		}
	}
	return out;
}

/** Add a command to the tree
 * Inject the given command into the tree, adjusting the position of existing
 * nodes as needed to maintain the correct structure of the tree.
 * @param name Command name
 * @param fact Pointer to command factory
 * @param id Command identifier to the factory
 * @return true if command was successfully injected, false otherwise.  We
 * return false if the command already exists in the command tree.
 * @note The command tree is case insensitive.
 * @todo We need to make sure that the shortest commands bubble to the top
 */
bool CommandTree::addcmd(QString name, CommandFactory *fact, unsigned int id)
{
	CommandTreeNode *newnode = new CommandTreeNode(name, fact, id);
	int len = name.length();
	CommandTreeNode *loc = rootnode;
	const char *cname = name.latin1();
	short edge;
	short specialedge = loc->edgenum(';');

	for (int i = 0; i< len; i++)
	{
		edge = loc->edgenum(cname[i]);

		/* non-alpha chars need special handling.  We need to go down the tree
		 * until we find an edge that matches the current char or is empty */
		if (edge == specialedge)
		{
			while (!(loc->_name[i] == cname[i]) && loc->edges[edge])
			{
				loc = loc->edges[edge];
			}
		}

		/* If the edge is null, we just put our new node into that spot */
		if (loc->edges[edge] == NULL)
		{
			loc->edges[edge] = newnode;
			newnode->parent = loc;
			if (edge == specialedge)
			{
				newnode->depth = i;
			} else {
				newnode->depth = i+1;
			}
			return true;
		}
		loc = loc->edges[edge];
		
		/* Make another try for fitting a short command into the tree */
		if (len < (int)loc->_name.length())
		{
			/* Swap positions */
			CommandTreeNode *tmp = newnode;
			newnode = loc;
			loc = tmp;

			/* Copy edges array and clear the edges array in the new newnode */
			for (unsigned int j = 0; j < CommandTreeNode::numedges; j++)
			{
				loc->edges[j] = newnode->edges[j];
				newnode->edges[j] = NULL;
				if (loc->edges[j])
				{
					loc->edges[j]->parent = loc;
				}
			}
			loc->parent = newnode->parent;
			loc->parent->edges[loc->edgenum(cname[i])] = loc;
			if (edge == specialedge)
			{
				loc->depth = i;
			} else {
				loc->depth = i+1;
			}

			/* Start placement search back at the top with the longer command */
			cname = newnode->_name.latin1();
			edge = newnode->edgenum(cname[0]);
			len = newnode->_name.length();
			loc = rootnode;
			i = -1;
		}
	}


	delete newnode;
	return false;
}

/** Find a command without abbreviations.  If the strings do not completely
 * match, we return NULL.
 * @param cmd String to find
 * @return Pointer to node with string or NULL if not found.
 */
CommandTree::CommandTreeNode *CommandTree::find_full(QString cmd)
{
	CommandTreeNode *loc = rootnode;
	bool done = false;
	unsigned short len;
	const char *ccmd = cmd.latin1();

	if ((len = cmd.length()) < 1)
	{
		return NULL;
	}
	
	short specialedge = loc->edgenum(';');
	short curedge = loc->edgenum(ccmd[loc->depth]);
	do
	{
		/* If the strings match, we're done */
		if (cmd == loc->_name ||
				(curedge == specialedge && cmd.left(loc->depth+1) == loc->_name
					&& (loc->edgenum(ccmd[loc->depth + 1]) != specialedge
							&& !loc->edges[loc->edgenum(ccmd[loc->depth + 1])])))
		{
			return loc;
		} else if (len == loc->depth) // we're at the end of cmd with no match
			break;

		/* If it doesn't match, we need to traverse down a level.  */
		curedge = loc->edgenum(ccmd[loc->depth]);
		if ((loc = loc->edges[curedge]) == NULL)
			break;
	} while (!done);
	
	return NULL;
}

/** Find a command or an abbreviation of a command.
 * We will always match the full command if possible.
 * @param cmd Command to search for
 * @return pointer to CommandTreeNode matching command
 */
CommandTree::CommandTreeNode *CommandTree::find_abbrev(QString cmd)
{
	CommandTreeNode *loc = rootnode;
	bool done = false;
	unsigned short len;
	const char *ccmd = cmd.latin1();

	if ((len = cmd.length()) < 1)
	{
		return NULL;
	}

	short specialedge = loc->edgenum(';');
	short curedge = loc->edgenum(ccmd[loc->depth]);
	do
	{
		/* If the strings match, we're done */
		if (cmd == loc->_name ||
				(curedge == specialedge && cmd.left(loc->depth+1) == loc->_name
					&& (loc->edgenum(ccmd[loc->depth + 1]) != specialedge
							&& !loc->edges[loc->edgenum(ccmd[loc->depth + 1])])))
		{
			return loc;
		} else if (loc->_name.startsWith(cmd)) { // abbreviation match
			return loc;
		} else if (len == loc->depth) // we're at the end of cmd with no match
			break;

		/* If it doesn't match, we need to traverse down a level.  */
		curedge = loc->edgenum(ccmd[loc->depth]);
		if ((loc = loc->edges[curedge]) == NULL)
			break;

	} while (!done);
	
	return NULL;
}

}; /* end koalamud namespace */
