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
	Editor *edit;

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
			if (--field >= 0 && field < fieldlist.count())
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
							curstate = STATE_EDITENUM;
							curstate = STATE_MAINMENU;
							break;
						case FIELD_SET:
							/* Need to send the field list menu */
							curstate = STATE_EDITSET;
							curstate = STATE_MAINMENU;
							break;
					}
				}
			}
			break;
		case STATE_EDITFIELD:
			line.truncate(line.length() - 2);
			/* Check that our input is in the appropriate range */
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
			curstate = STATE_MAINMENU;
			dosendmenu = true;
			break;
		case STATE_EDITENUM:
			/* Check that our input is in the appropriate range */
			if (line.toLong() < curfield->min)
			{
				os << endl << curfield->name << " requires a number greater than "
					 << curfield->min << "." << endl;
			} else if (line.toLong() > curfield->max)
			{
				os << endl << curfield->name << " requires a number less than "
					 << curfield->max << "." << endl;
			} else {
				*(curfield->numeric) = line.toLong();
				os << endl << curfield->name << " has been set." << endl;
			}
			curstate = STATE_MAINMENU;
			dosendmenu = true;
			break;
		case STATE_EDITSET:
			/* We will stay in this state for more than one line of input, so we
			 * won't always send the main menu from here.
			 * FIXME:  Implement this.
			 */
			dosendmenu = true;
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
				for (listnode_t *curnode = cur->flaglist.first(); curnode;
							curnode = cur->flaglist.next())
				{
					unsigned int count = 0;
					if ((*(cur->numeric)) & curnode->flagnum)
					{
						if (count)
						{
							os << ", ";
						}
						os << curnode->name;
						count++;
					}
				}
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

/** Escape special characters in @a str for push into mysql
 * This will escape special characters like \ and ' with a \
 */
QString olc::escapeString(QString str)
{
	QString out;
	out = str.replace(QRegExp("\\"), "\\\\");
	out = out.replace(QRegExp("'"), "\\'");
	return out;
}

}; /* end koalamud namespace */
