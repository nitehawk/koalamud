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

#define KOALA_EDITOR_CXX "%A%"

#include "editor.hxx"
#include "language.hxx"

namespace koalamud
{

/** Start editing from a command
 * @param ch Character to attach editor to
 * @param pd Descriptor to attach editor to
 * @param oldParser Parser to return control to when we are done
 * @param postcmd command to pass edited string back to when done
 * @param initial Initial editor state
 * @param sendinitial Do we send the help screen when we load
 */
Editor::Editor(Char *ch, ParseDescriptor *pd, Parser *oldParser,
						Command *postcmd, QString initial = "", bool sendinitial=true)
		: Parser(ch, pd), isOLC(false), _old(oldParser), _postcmd(postcmd),
			curstate(STATE_MENU)
{
	QTextOStream elos(&el);
	elos << endl;
	lines = QStringList::split(el, initial, true);

	if (sendinitial)
	{
		sendHelp();
	}
}

/** Start editing from OLC
 * @param ch Character to attach editor to
 * @param pd Descriptor to attach editor to
 * @param activeolc OLC we were called from
 * @param initial Initial editor state
 * @param sendinitial Do we send the help screen when we load
 */
Editor::Editor(Char *ch, ParseDescriptor *pd, olc *activeolc,
								QString initial="", bool sendinitial=true)
		: Parser(ch, pd), isOLC(true), _old(activeolc), _postcmd(NULL),
			curstate(STATE_MENU)
{
	QTextOStream elos(&el);
	elos << endl;
	lines = QStringList::split(el, initial, true);

	if (sendinitial)
	{
		sendHelp();
	}
}

/** Return control to calling context
 * We split this out of parseLine to save subclasses from needing to implement
 * this
 */
void Editor::returnControl(bool aborting=false)
{
	QString complete = lines.join(el);
	if (isOLC)
	{
		if (aborting)
		{
			_old->parseLine("abort");
		} else {
			_old->parseLine(complete);
		}
		_desc->setParser(_old);
	} else {
		if (aborting)
		{
			_postcmd->run("abort");
		} else {
			_postcmd->run(complete);
		}
		delete _postcmd;
		_old->parseLine(QString(""));
		_desc->setParser(_old);
	}
}

/** Parse a line of input
 * Since we only need to worry about state in this function, we define our
 * states as a local type.
 */
void Editor::parseLine(QString line)
{
	QString cline = line.simplifyWhiteSpace();
	QString out;
	QTextOStream os(&out);

	switch (curstate)
	{
		case STATE_MENU:
			if (QString("@append").startsWith(cline.lower()))
			{
				os << endl <<
"Appending to buffer.  Enter . on separate line to return to menu." << endl;
				_desc->send(out);
				curstate = STATE_APPEND;
				break;
			}
			if (QString("@insert").startsWith(cline.section(" ",0,0).lower()))
			{
				insertpos = cline.section(" ", 1,1).toInt();
				if (insertpos > 0)
					--insertpos;
				os << endl <<
"Inserting into buffer.  Enter . on separate line to return to menu." << endl;
				_desc->send(out);
				curstate = STATE_INSERT;
				break;
			}
			if (QString("@delete").startsWith(cline.section(" ", 0, 0).lower()))
			{
				bool ok;
				int line1 = cline.section(" ", 1,1).toInt(&ok) - 1;
				if (!ok)
				{
					os << endl << "Need to specify a line or range to delete." << endl;
					_desc->send(out);
					break;;
				}
				int line2 = cline.section(" ", 2,2).toInt(&ok) - 1;
				if (ok)
				{
					lines.erase(lines.at(line1), lines.at(line2));
					os << endl << line2 - line1
						 << " lines deleted.  Type @list to see updated buffer." << endl;
					_desc->send(out);
					break;
				} else {
					os << endl << "1 line deleted.  Type @list to see updated buffer." << endl;
					_desc->send(out);
					lines.erase(lines.at(line1));
					break;
				}
			
			}
			if (QString("@help").startsWith(cline.lower()))
			{
				sendHelp();
				break;
			}
			if (QString("@language").startsWith(cline.lower()))
			{
				QString lang = cline.section(" ", 1,1);
				lang = Language::getLangIDfromShort(lang);
				if (lang.length() != 5)
				{
					os << endl << "That language is unknown." << endl;
					_desc->send(out);
					break;
				}
				os << endl << "Starting soon, this will allow you to change the"
					<< " language for a part of an editor string.  This editor"
					<< " feature is currently unimplemented." << endl;
				_desc->send(out);
				break;
			}
			if (QString("@list").startsWith(cline.lower()))
			{
				QStringList::iterator cur;
				os << endl;
				int count = 1;
				for (cur = lines.begin(); cur != lines.end(); ++cur)
				{
					os.width(3);
					os << count++;
					os.width(0);
					os << ". " << *cur << endl;
				}
				os << endl;
				_desc->send(out);
				break;
			}
			if (QString("@quit").startsWith(cline.lower()))
			{
				returnControl();
				break;
			}
			if (QString("@abort").startsWith(cline.lower()))
			{
				returnControl(true);
				break;
			}
			sendHelp();
			break;
		case STATE_APPEND:
			if (line.stripWhiteSpace() == ".")
			{
				curstate = STATE_MENU;
				break;
			}
			/* Append a line to the end of the edit buffer */
			lines += line;
			break;
		case STATE_INSERT:
			if (line.stripWhiteSpace() == ".")
			{
				curstate = STATE_MENU;
				break;
			}
			/* Insert line before insertpos - increment insertpos */
			lines.insert(lines.at(insertpos++), line);
			break;
	}
}

/** Send help menu
 * This should send a menu of how to work with the editor.
 */
void Editor::sendHelp(void)
{
	QString out;
	QTextOStream os(&out);

	os << endl <<
"KoalaMud Text Editor" << endl <<
"@quit     Save and quit" << endl <<
"@abort    Abort" << endl <<
"@list     List edit buffer" << endl <<
"@language Set language to be used:" << endl <<
"            takes shortname, startline, endline" << endl <<
"            startline and endline can be left off to start" << endl <<
"            appending to the buffer in a specific language" << endl <<
"@h        This help page" << endl << 
"@i #      Insert lines before #" << endl <<
"@a        Append lines to end of buffer" << endl <<
"@d #      Delete line number #" << endl <<
"@d #1 #2  Delete lines #1 to #2" << endl <<
"While editing, a . on its own will bring you back to the menu." << endl;
_desc->send(out);
}

}; /* end koalamud namespace */
