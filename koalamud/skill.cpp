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

#define KOALA_SKILL_CXX "%A%"

#include <qdict.h>
#include <qstring.h>

#include "skill.hxx"
#include "cmd.hxx"
#include "cmdtree.hxx"
#include "playerchar.hxx"

namespace koalamud
{

/** Map skill ids to their objects */
QDict<Skill> skillmap;

/** Set id and name */
Skill::Skill(QString skid, QString skname)
		: _id(skid), _name(skname)
{
	skillmap.insert(skid, this);
}

/** Lookup a skill in the skill tables */
Skill *Skill::getSkill(QString id)
{
	return skillmap[id];
}

namespace commands
{

/** Skills command class */
class Skills : public Command
{
	public:
		/** Pass through constructor */
		Skills(Char *ch) : Command(ch) {}
		/** Run Skills command */
		virtual unsigned int run(QString args)
		{
			QString out;
			QTextOStream os(&out);
			QDictIterator<SkillRecord> skcur = _ch->getSkrecIter();

			os << "You have knowledge of the following Skills:" << endl
				 << "Skill Name              Base level   Bonus    Total knowledge"
				 << endl;

			os.setf(QTextStream::left);
			for (skcur.toFirst(); *skcur; ++skcur)
			{
				os.width(24);
				os << (*skcur)->getRecord()->getName();
				os.width(13);
				os << (*skcur)->getKnow();
				os.width(9);
				os << (*skcur)->getBonus();
				os << (*skcur)->getLev() << endl;
			}

			os << endl;

			_ch->sendtochar(out);
			return 0;
		}

};

/** Skill Set command class */
class SkillSet : public Command
{
	public:
		/** Pass through constructor */
		SkillSet(Char *ch) : Command(ch) {}
		/** Run Skill Set command */
		virtual unsigned int run(QString args)
		{
			QString out;
			QTextOStream os(&out);

			QString tgtname = args.section(' ', 0, 0);
			QString skid = args.section(' ', 1, 1);
			int lvl = args.section(' ', 2, 2).toInt();
			
			Char *tgt = connectedplayermap[tgtname];
			if (tgt == NULL)
			{
				os << endl << "That player is not online." << endl;
				_ch->sendtochar(out);
				return 1;
			}

			if (skid.length() != 5)
			{
				os << endl << "You must specify a valid skill ID." << endl;
				_ch->sendtochar(out);
				return 1;
			}

			if (!tgt->setSkillLevel(skid, lvl))
			{
				os << endl << "You must specify a valid skill ID." << endl;
				_ch->sendtochar(out);
				return 1;
			}

			os << endl << "Successfully set " << tgtname << "'s skill in " << skid
				 << " to " << lvl << endl;
			_ch->sendtochar(out);

			QString tgtout;
			QTextOStream tos(&tgtout);

			tos << endl << "The gods have granted you knowledge in "
					<< Skill::getSkill(skid)->getName() << "." << endl;
			
			tgt->sendtochar(tgtout);
			tgt->sendPrompt();
			return 0;
		}

		/** Restricted access command. */
		virtual bool isRestricted(void) const { return true;}

		/** Command Groups */
		virtual QStringList getCmdGroups(void) const
		{
			QStringList gl;
			gl << "Implementor";
			return gl;
		}

		/** Get command name for individual granting */
		virtual QString getCmdName(void) const { return QString("skillset"); }
};

}; /* end commands namespace */

/** Command Factory for skill.cpp */
class Skill_CPP_CommandFactory : public CommandFactory
{
	public:
		/** Register our commands */
		Skill_CPP_CommandFactory(void)
			: CommandFactory()
		{
			immcmdtree->addcmd("skillset", this, 1);
			maincmdtree->addcmd("skills", this, 2);
		}

		/** Handle command object creations */
		virtual Command *create(unsigned int id, Char *ch)
		{
			switch (id)
			{
				case 1:
					return new koalamud::commands::SkillSet(ch);
				case 2:
					return new koalamud::commands::Skills(ch);
			}
			return NULL;
		}
};

/** Command factory for skill.cpp module.  */
Skill_CPP_CommandFactory Skill_CPP_CommandFactoryInstance;

}; /* end koalamud namespace */
