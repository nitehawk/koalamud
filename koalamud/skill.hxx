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

#ifndef KOALA_SKILL_HXX
#define KOALA_SKILL_HXX "%A%"

#include <qstring.h>

namespace koalamud {

/** Very basic skill information
 * This exists primarily to provide a base class for all the skills to use so
 * they can be put into lists together.  The main skill types will have their
 * own base class with this as a parent.
 */
class Skill
{
	public:
		Skill(QString skid, QString skname);
		static Skill *getSkill(QString id);
	
		/** Get skill Name */
		QString getName(void) const { return _name; }
		/** Get skill ID */
		QString getID(void) const { return _id; }
	protected:
		/** Skill ID */
		QString _id;
		/** Full skill name */
		QString _name;
};

/** This is used in the Char class tree to provide a mapping from skillID to
 * the information for the skill.  This includes things like base rating and
 * bonuses as well.
 */
class SkillRecord
{
	public:
		/** Setup a skill record */
		SkillRecord(QString id, int know, int bonus, Skill *record)
			: _id(id), knowlevel(know), knowbonus(bonus), skrec(record)
			{}

	public:
		/** Get skill ID */
		QString getId(void) const {return _id;}
		/** Get current know level */
		int getKnow(void) const {return knowlevel;}
		/** Get know bonus */
		int getBonus(void) const {return knowbonus;}
		/** Get skill level - knowlevel + bonus */
		int getLev(void) const { return knowlevel + knowbonus; }
		/** Get skill record */
		Skill *getRecord(void) const { return skrec; }
		/** Set skill bonus - Some items provide knowledge bonuses, and language
		 * skills get a bonus based on the parent language know levels - return
		 * the new bonus */
		int setBonus(int newbonus) { knowbonus = newbonus; return knowbonus;}
		/** Set skill know level */
		int setKnow(int newknow) { knowlevel = newknow; return knowlevel;}
		
	protected:
		/** Skill ID */
		QString _id;
		/** Base skill level */
		int knowlevel;
		/** Know bonus */
		int knowbonus;
		/** Pointer to the skill record */
		Skill *skrec;
};

}; /* end koalamud namespace */

#endif //  KOALA_SKILL_HXX
