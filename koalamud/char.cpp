/***************************************************************\
*                        KoalaMud Gen 2                         *
*    Copyright (c) 2002 First Step Internet Services, Inc.      *
*                     All Rights Reserved                       *
*        Distributed under the terms of the FSI License         *
* See file LICENSE in the root of this package for information  *
\***************************************************************/
/***************************************************************\
*	Module:
* Description:
* Classes:
\***************************************************************/

#define KOALA_CHAR_CXX "%A%"

#include <qregexp.h>

#include "main.hxx"
#include "char.hxx"
#include "event.hxx"
#include "language.hxx"

namespace koalamud
{
/** Name supplied in constructor */
Char::Char(QString name = NULL, ParseDescriptor *desc=NULL)
	: _name(name), _desc(NULL), _disconnecting(false),
		cmdtaskrunning(false), _inroom(NULL)
{
	if (desc)
		setDesc(desc);
}

/** Make sure that we leave the room we are in */
Char::~Char(void)
{
	if (_inroom)
		_inroom->leaveRoom(this);
}


/** Get the characters name.
 * If @a pair is specified, this will check if the character is visible to the
 * character pointed to by @a pair.  If they are visible, it will next check
 * to see if they know each other.
 * @note getVoice (currently unimplemented) will return a string for the voice
 * of a character.   getVoice should be used in all comm functions.  getName
 * should be used anywhere where you would 'see' the character (listed in
 * room, looking at character, etc.)
 */
QString Char::getName(Char *pair=NULL)
{
	if (pair == NULL)
		return _name;

	/* Check visibility - Simple for now */
	if (visibleTo(pair))
	{
		return _name;
	} else {
		return "someone";
	}
}

/** Reset disconnecting status */
void Char::setdisconnect(bool dis = true)
{
	_disconnecting = dis;
	_desc->markClose();
}

/** Is this character visible to the @a pair character. */
bool Char::visibleTo(Char *pair)
{
	return true;
}

/** Get the short description for a player
 * This is used primarily in displaying rooms
 */
QString Char::getShortDesc(Char *pair=NULL)
{
	QString out;
	QTextOStream os(&out);
	os << _name << " is resting here.";
	return out;
}

/** Attach a descriptor to character */
void Char::setDesc(ParseDescriptor *desc)
{
	/* Disconnect all signals from this descriptor */
	if (_desc)
	{
		disconnect(_desc);
	}
	_desc = desc;
	/* Connect close signals */
	connect(desc, SIGNAL(destroyed()), this, SLOT(descriptorClosed()));
}

/** Send a message from a channel on to the char */
void Char::channelsendtochar(Char *from, QString templateall,
			QString templatesender, QString msg)
{
	if (from == this)
	{
		/* This is the version to ourself, fill in template sender and send it on
		 */
		QString outmsg;
		outmsg = templatesender.replace(QRegExp("%sender%"), "You");
		outmsg = outmsg.replace(QRegExp("%message%"), msg);
		QString endline;
		QTextOStream os(&endline);
		os << endl;
		sendtochar(endline);
		sendtochar(outmsg);
		sendtochar(endline);
		return;
	} else
	{
		QString outmsg;
		if (from != NULL)
		{
			outmsg = templateall.replace(QRegExp("%sender%"),
						from->getName(this) );
		}
		outmsg = outmsg.replace(QRegExp("%message%"), msg);
		QString endline;
		QTextOStream os(&endline);
		os << endl;
		sendtochar(endline);
		sendtochar(outmsg);
		sendtochar(endline);
		sendPrompt();
	}
}

/** Add a command to the command queue
 * Also make sure a task is in existance to eventually run it.
 */
void Char::queueCommand(Command *cmd, QString args)
{
	cmdqueueitem *newitem = new cmdqueueitem;
	newitem->cmd = cmd;
	newitem->args = args;

	cmdqueuelock.acquire();
	cmdqueue.enqueue(newitem);
	if (!cmdtaskrunning)
	{
		cmdtaskrunning = true;
		/* Queue task */
		srv->executor()->execute(new cmdexectask(this));
	}
	cmdqueuelock.release();
}

bool Char::sendtochar(QString data)
{
	{
	/* Scan string for language markers - send off to be garbled
	 * Language markup looks like:  ^&LANG [langid] text GNAL&^
	 * In the event that GNAL&^ is not found, we will use the end of the string
	 * as our marking point.  Language markers cannot be be nested.
	 */
		int start,end,msgstart,msgend;
		while ((start = data.find("^&LANG")) >= 0)
		{
			msgend = data.find("GNAL&^", start) - 1;
			end = msgend + strlen("GNAL&^") + 1;
			msgstart = start + strlen("^&LANG") + 1;

			/* Extract the language ID and move msgstart */
			/* Language IDs are 5 character strings that are mapped to the language
			 * objects.  Language names are mapped to language IDs.  These mappings
			 * are static so that descriptions can include language markers.
			 */
			QString langid = data.mid(msgstart, 5);
			msgstart += 6;

			QString msg = data.mid(msgstart, msgend - msgstart);

			/* Update output string with morphed string */
			data = data.replace(start, end - start, languageMorph(langid,msg));
		}
	}

	if (_desc)
	{
		_desc->send(data);
		return true;
	}
	return false;
}

/** Morph a string based on the target language.
 * This function wraps translating langid into the appropriate language object
 * and skill rating.
 * @note This function can also be used to premorph strings before sending to
 * other players to take into account the players skill in speaking the
 * language by specifying true to spoken.  This will morph to the players
 * primary language with a skill rating of the desired language.  Thus when it
 * goes out, the string will be flavored with the characters primary language
 * if they don't have the language skill maxed out.
 */
QString Char::languageMorph(QString langid, QString msg, bool spoken = false)
{
	Language *lang;
	int know;
	if (spoken)
	{
		lang = priLanguage;
		if (lang == NULL)
		{
			/* For now make elven the default if no language is set */
			lang = priLanguage = Language::getLanguage("elven");
		}
		know = getKnow(langid);
	} else {
		lang = Language::getLanguage(langid);
		know = getKnow(langid);
	}
	if (lang)
		return lang->morphString(msg, know);
	else
		return msg;
}

/** Get the complete know level for a skill */
int Char::getKnow(QString id)
{
	SkillRecord *skrec = skills[id];
	if (skrec == NULL)
		return 0;

	return skrec->getLev();
}

/** Set the skill level for specified skill.  If they don't already know this
 * skill, add it to their skill list. */
bool Char::setSkillLevel(QString id, int level)
{
	SkillRecord *skrec = skills[id];
	if (skrec == NULL)
	{
		Skill *newskill;
		newskill = Skill::getSkill(id);
		if (newskill == NULL)
		{
			return false;
		}
		skrec = new SkillRecord(id, level, 0, newskill);
		skills.insert(id, skrec);
	} else {
		skrec->setKnow(level);
	}
	return true;
}

/* {{{ cmdexectask implementation */
/** Initialize a command execution task */
void Char::cmdexectask::run(void)
{
	/* Don't do anything if we are are a player and disconnecting */
	if (_ch->isDisconnecting() && _ch->isPC())
		return;

	/* Get the first pending command */
	_ch->cmdqueuelock.acquire();
	cmdqueueitem *cmditem = _ch->cmdqueue.dequeue();
	_ch->cmdqueuelock.release();

	/* run command */
	try {
		cmditem->cmd->runCmd(cmditem->args);
	}
	catch (koalamud::exceptions::cmdpermdenied p)
	{
		QString out;
		QTextOStream os(&out);
		os << "You do not have permission to run this command." << endl;
		_ch->sendtochar(out);
	}
	_ch->sendPrompt();

	/* cleanup */
	delete cmditem->cmd;
	delete cmditem;
	
	/* Don't schedule any more command executors if we are disconnecting */
	if (_ch->isDisconnecting())
		return;

	/* If there are more commands pending, start another executor */
	_ch->cmdqueuelock.acquire();
	if (_ch->cmdqueue.isEmpty())
	{
		_ch->cmdtaskrunning = false;
	} else {
		srv->executor()->execute(new cmdexectask(_ch));
	}
	_ch->cmdqueuelock.release();
}

/* }}} end cmdexectask implementation */

}; /* End koalamud namespace */
