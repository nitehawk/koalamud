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

#include "char.hxx"

K_Char::K_Char()
{
}
/** K_Char DestructorBar */
 K_Char::~K_Char()
{
}

void K_Char::setName(QString name)
{
	_name = name;
}

QString K_Char::getName(K_Char *pair=NULL)
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

bool K_Char::visibleTo(K_Char *pair)
{
	return true;
}
