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

#define KOALA_OLC_CXX "%A%"

#include <qregexp.h>

#include "olc.hxx"
#include "editor.hxx"

namespace koalamud
{
	
/** Construct an OLC object.
 * Since we don't have all the information we need until we are completely
 * constructed, we need the calling class to call in to display the menu.
 * olc->sendMenu();
 */
olc::olc(Char *ch, ParseDescriptor *pd, Parser *oldParser)
	: Parser(ch, pd), _old(oldParser)
{
	fieldlist.setAutoDelete(true);
}

/** Parse a line of input
 * Depending on our state, this is either selecting an item from a menu,
 * setting a string object, saving, quitting or aborting editing (abort only
 * aborts to the last save).
 */
void olc::parseLine(QString line)
{
	QString out;
	QTextOStream os(&out);
	QString cline = line.simplifyWhiteSpace();
	int field;
	bool dosendmenu = false;
	bool dosendout = true;
	Editor *edit;
	bool valueok;
	unsigned int value = cline.toUInt(&valueok);

	switch (curstate)
	{
		case STATE_MAINMENU:
			/* Check input for keywords, then check to see if it matches an editable
			 * field */
 			field = cline.toInt();
			if (QString("refresh").startsWith(cline.lower()))
			{
				dosendmenu = true;
				break;
			}
			if (QString("save").startsWith(cline.lower()))
			{
				save();
				break;
			}
			if (QString("shell").startsWith(cline.lower()))
			{
				_old->parseLine(cline.section(' ', 1));
				break;
			}
			if (QString("abort").startsWith(cline.lower()))
			{
				os << "Aborting OLC" << endl;
				_desc->send(out);
				_old->parseLine(QString(""));
				_desc->setParser(_old);
				return;
			}
			if (QString("quit").startsWith(cline.lower()))
			{
				os << "Saving and quiting OLC" << endl;
				_desc->send(out);
				save();
				_old->parseLine(QString(""));
				_desc->setParser(_old);
				return;
			}
			if (--field >= 0 && field < (int)fieldlist.count())
			{
				curfield = fieldlist.at(field);
				if (curfield->editable)
				{
					switch (curfield->type)
					{
						case FIELD_STRING:
						case FIELD_INTEGER:
							curstate = STATE_EDITFIELD;
							os << endl << "Editing " << curfield->name << ".  New value: ";
							break;
						case FIELD_MULTILINE:
							curstate = STATE_EDITML;
							os << endl << "Editing " << curfield->name << "." << endl;
							_desc->send(out);
							edit = new Editor(_ch, _desc, this, *(curfield->str));
							_desc->setParser(edit, false);
							return;
						case FIELD_ENUM:
							/* Need to set the field list menu */
							_desc->send(buildFlagMenu(curfield));
							curstate = STATE_EDITENUM;
							break;
						case FIELD_SET:
							_desc->send(buildFlagMenu(curfield));
							/* Need to send the field list menu */
							curstate = STATE_EDITSET;
							break;
					}
				}
			}
			break;
		case STATE_EDITFIELD:
			/* Check that our input is in the appropriate range */
			switch (curfield->type)
			{
				case FIELD_STRING:
					if (curfield->min > 0 && line.length() < curfield->min)
					{
						os << endl << curfield->name << " requires a string at least "
							 << curfield->min << " characters long." << endl;
					} else if (curfield->max > 0 && line.length() > curfield->max)
					{
						os << endl << curfield->name << " requires a string at most "
							 << curfield->max << " characters long." << endl;
					} else {
						*(curfield->str) = line;
						os << endl << curfield->name << " has been set." << endl;
					}
					break;
				case FIELD_INTEGER:
					if (curfield->min > 0 && line.length() < curfield->min)
					{
						os << endl << curfield->name << " requires a number greater than "
							 << curfield->min << "." << endl;
					} else if (curfield->max > 0 && line.length() > curfield->max)
					{
						os << endl << curfield->name << " requires a number less than "
							 << curfield->max << "." << endl;
					} else {
						*(curfield->numeric) = line.toLong();
						os << endl << curfield->name << " has been set." << endl;
					}
					break;
				default:
					break;
			}
			curstate = STATE_MAINMENU;
			dosendmenu = true;
			break;
		case STATE_EDITENUM:
			/* Check our input for an unsigned int.  If we have one, match it to a
			 * flag or redisplay the enum menu. */
			if (valueok)
			{
				for (listnode_t *curnode = curfield->flaglist.first(); curnode;
							curnode = curfield->flaglist.next())
				{
					if (curnode->flagnum == value)
					{
						*(curfield->numeric) = value;
						os << endl << curfield->name << " has been set to "
							 << curnode->name << endl;
						curstate = STATE_MAINMENU;
						dosendmenu = true;
						break;
					}
				}
			}
			os << "'" << line << "' is not a valid selection." << endl;
			_desc->send(out);
			dosendout = false;
			_desc->send(buildFlagMenu(curfield));
			break;
		case STATE_EDITSET:
			/* We stay in editset mode until we get a 'quit' */
			if (QString("quit").startsWith(cline.lower()))
			{
				curstate = STATE_MAINMENU;
				dosendmenu = true;
				os << endl << curfield->name << " has been updated." << endl;
				break;
			}
			if (valueok)
			{
				listnode_t *curnode = NULL;
				for (curnode = curfield->flaglist.first(); curnode;
							curnode = curfield->flaglist.next())
				{
					if (curnode->flagnum == value)
					{
						*(curfield->numeric) ^= 1 << value;
						dosendout = false;
						_desc->send(buildFlagMenu(curfield));
						break;
					}
				}
				if (!curnode)
				{
					os << "'" << line << "' is not a valid selection." << endl;
					_desc->send(out);
					dosendout = false;
					_desc->send(buildFlagMenu(curfield));
				}
			} else {
				os << "'" << line << "' is not a valid selection." << endl;
				_desc->send(out);
				dosendout = false;
				_desc->send(buildFlagMenu(curfield));
			}
			break;
		case STATE_EDITML:
			curstate = STATE_MAINMENU;
			if (line == "abort")
			{
				os << "Editor aborted" << endl;
			} else {
				*(curfield->str) = line;
			}
			dosendmenu = true;
			break;
	}
	if (dosendout)
		_desc->send(out);
	if (dosendmenu)
		sendMenu();
}

/** Send main OLC menu
 * This sends a menu of options for a particular OLC.  The menu is generated
 * based on variables added to a list by a subclass and is automatically
 * arranged as needed to fit as well as possible.
 */
void olc::sendMenu(void)
{
	field_t *cur;
	unsigned int field = 1;
	QString out;
	QTextOStream os(&out);

	os << endl;

	for (cur = fieldlist.first(); cur; cur = fieldlist.next())
	{
		/* Output field number */
		os.width(4);
		if (cur->editable)
			os << field << ".";
		else
			os << " ";
		field++;

		/* Output Field Name */
		os.width(0);
		os << cur->name << ": ";

		/* Output field value */
		switch (cur->type)
		{
			case FIELD_STRING:
				os << *(cur->str) << endl;
				break;
			case FIELD_INTEGER:
				os << *(cur->numeric) << endl;
				break;
			case FIELD_MULTILINE:
				os << "Multiline field" << endl;
				break;
			case FIELD_ENUM:
				/* Loop through the list attached to cur until we find a field with
				 * the correct value */
				for (listnode_t *curnode = cur->flaglist.first(); curnode;
							curnode = cur->flaglist.next())
				{
					if (curnode->flagnum == *(cur->numeric))
					{
						os << curnode->name << endl;
						break;
					}
				}
				break;
			case FIELD_SET:
				/* Loop through the attached list and output each record that is set
				 */
				{
					unsigned int count = 0;
					for (listnode_t *curnode = cur->flaglist.first(); curnode;
								curnode = cur->flaglist.next())
					{
						if ((*(cur->numeric)) & (1 << curnode->flagnum))
						{
							if (count)
							{
								os << ", ";
							}
							os << curnode->name;
							count++;
						}
					}
				}
				os << endl;
				break;
		}

	}
	_desc->send(out);
}

/** Add an OLC item to the list
 * Create a field_t object and add it to the field list.
 * @param name Field name
 * @param type Field type.  See definition of fieldtype_t.
 * @param strval Pointer to QString used to store value
 * @param editable Can this field be edited.
 * @param max Maximum string length acceptable (default 0 = no max)
 * @param min Minimum string length acceptable (default 0 = no min)
 * @return pointer to the newly created field (used in enum/set item list
 * adition).
 */
olc::field_t *olc::addField(QString name, fieldtype_t type,
									QString *strval, bool editable, long max=0, long min=0)
{
	field_t *newfield = new field_t;
	newfield->name = name;
	newfield->type = type;
	newfield->str = strval;
	newfield->min = min;
	newfield->max = max;
	newfield->editable = editable;

	fieldlist.append(newfield);

	return newfield;
}

/** Add an OLC item to the list
 * Create a field_t object and add it to the field list.
 * @param name Field name
 * @param type Field type.  See definition of fieldtype_t.
 * @param numval Pointer to long integer used to store value.
 * @param editable Can this field be edited.
 * @param max Maximum value acceptable (default 0)
 * @param min Minimum value acceptable (default 0)
 * @return pointer to the newly created field (used in enum/set item list
 * adition).
 */
olc::field_t *olc::addField(QString name, fieldtype_t type,
									long *numval, bool editable, long max=0, long min=0)
{
	field_t *newfield = new field_t;
	newfield->name = name;
	newfield->type = type;
	newfield->numeric = numval;
	newfield->min = min;
	newfield->max = max;
	newfield->editable = editable;

	fieldlist.append(newfield);

	return newfield;
}

/** Add a flag to an Enum or Set field type */
void olc::addFlag(olc::field_t *field, unsigned int flag, QString name)
{
	if (field->type == FIELD_ENUM || field->type == FIELD_SET)
	{
		listnode_t *newflag = new listnode_t;
		newflag->flagnum = flag;
		newflag->name = name;
		field->flaglist.append(newflag);
		field->flaglist.setAutoDelete(true);
	}
}

/** Build a QString with a menu built for an enum or set field
 * For set fields, we will mark each item that is selected. */
QString olc::buildFlagMenu(olc::field_t *field)
{
	/* Make sure we are dealing with the right field type */
	if (field->type != FIELD_ENUM && field->type != FIELD_SET)
		return "";

	bool markset = (field->type == FIELD_SET);

	QString out;
	QTextOStream fos(&out);

	fos << (markset ? "Set: " : "Enum: ") << field->name << endl << endl;
	
	listnode_t *cur;
	for (cur = field->flaglist.first(); cur; cur = field->flaglist.next())
	{
		fos.width(3);
		fos << cur->flagnum;
		fos.width(0);
		if (markset && (*(field->numeric) & (1 << cur->flagnum)) )
		{
			fos << " * ";
		} else {
			fos << "   ";
		}

		fos << cur->name << endl;
	}

	fos << endl;

	return out;
}

namespace commands {
/** CommandList command class */
class Olc : public Command
{
	public:
		/** Pass through constructor */
		Olc(Char *ch) : Command(ch) {}
		/** Run Olc command */
		virtual unsigned int run(QString args)
		{
			koalamud::Command *subcmd = NULL;

			/* Get subcommand word */
			QString word = args.section(' ', 0, 0);
			
			/* Lookup the subcommand on the logging subcommand tree */
			subcmd = olccmdtree->findandcreate(word, _ch, true);

			if (subcmd == NULL && word.length() > 1)
			{ /* Oops, invalid subcommand */
				QString str;
				QTextOStream os(&str);
				os << "Invalid OLC command."
				   << endl;
				_ch->sendtochar(str);
				return 1;
			} else if (subcmd == NULL) {
				/* No subcommand specified */
				QString str;
				QTextOStream os(&str);
				os << "You must specify an olc command."
				   << endl;
				_ch->sendtochar(str);
				return 1;
			}

			/* Run the subcommand */
			return subcmd->runCmd(args.section(' ', 1));

			return 0;
		}

		/** Restricted access command. */
		virtual bool isRestricted(void) const { return true;}

		/** Command Groups */
		virtual QStringList getCmdGroups(void) const
		{
			QStringList gl;
			gl << "Implementor" << "Builder" << "Immortal";
			return gl;
		}

		/** Get command name for individual granting */
		virtual QString getCmdName(void) const { return QString("olc"); }
};

}; /* end namespace command */

/** Command Factory for cmd.cpp */
class Olc_CPP_CommandFactory : public CommandFactory
{
	public:
		/** Register our commands */
		Olc_CPP_CommandFactory(void)
			: CommandFactory()
		{
			immcmdtree->addcmd("olc", this, 1);
		}

		/** Handle command object creations */
		virtual Command *create(unsigned int id, Char *ch)
		{
			switch (id)
			{
				case 1:
					return new koalamud::commands::Olc(ch);
			}
			return NULL;
		}
};

/** Command factory for cmd.cpp module.  */
Olc_CPP_CommandFactory Olc_CPP_CommandFactoryInstance;

}; /* end koalamud namespace */
