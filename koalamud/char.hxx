/***************************************************************\
*                        KoalaMud Gen 2                         *
*    Copyright (c) 2002 First Step Internet Services, Inc.      *
*                     All Rights Reserved                       *
*        Distributed under the terms of the FSI License         *
* See file LICENSE in the root of this package for information  *
\***************************************************************/
/***************************************************************\
*	Module: CHAR
* Description:  Character baseclass used for both NPC and PC characters
* Classes: K_Char
\***************************************************************/

#ifndef KOALA_CHAR_HXX
#define KOALA_CHAR_HXX "%A%"

#include <qobject.h>

class K_Char : public QObject 
{
	Q_OBJECT

	public:
      K_Char();
      virtual  ~K_Char();

	public:
		virtual void setName(QString name);
		virtual QString getName(void) {return _name;}

	protected:
		QString _name;
};

#endif  // KOALA_CHAR_HXX
