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

#ifndef KOALA_DATABASE_HXX
#define KOALA_DATABASE_HXX "%A%"

#include <qsqldatabase.h>

namespace koalamud {

	/** Database interface module
	 * Since we are using Qt's built in SQL support, this class mainly exists to
	 * startup and shut down our database connection.  It may be desirable to
	 * run queries through the class, though that would add considerable
	 * complexity.
	 */
	class Database
	{
		public:
			Database(QString user="koalamud", QString pass="k23hjdsav",
							 QString db="koalamud", QString server="localhost",
							 QString driver="QMYSQL3");
			~Database();

			/** Return database status */
			bool isonline(void) { return dbonline; }
			QValueList<int> getListenPorts(QString profile);

		protected:
			/** Flag to track db status during startup */
			bool dbonline;
			/** Pointer to our database */
			QSqlDatabase *defaultDB;

			void checkschema(void);
	};

}; /* end koalamud namespace */

#endif //  KOALA_DATABASE_HXX
