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

#define KOALA_DATABASE_CXX "%A%"

#include <qsqldatabase.h>

#include "database.hxx"

/* SQL server login information */
#define SQLDRIVER "QMYSQL3"
#define SQLHOST "localhost"
#define SQLUSER "koalamud"
#define SQLPASS "k23hjdsav"
#define SQLDB "koalamud"

bool checkandupgradeschema(void)
{
	int schemaversion = 0;
	QSqlQuery query;

	QSqlQuery getschemaver("select vval from config where vname='SchemaVersion';");
	if (getschemaver.isActive())
	{
		getschemaver.next();
		schemaversion = getschemaver.value(0).toInt();
	}

	/* Each line in this case statement upgrades the schema to the next version.
	 * The last case takes no action as the latest version.
	 * {{{ Notes
	 * NOTE:  There are *NO* break statements between cases.
	 * NOTE:  This list *is* very order sensitive.  Changing the order of items
	 * *will* break schema upgrades
	 * NOTE:  Since the overall schema version number really doesn't have an
	 * upper limit, and to limit possible failure modes for upgrades, it is
	 * probably best to limit each schema version upgrade to two queries:  one
	 * to update table structures, one to update the schema version.  Having a
	 * group of updates that require multiple schema version updates really
	 * won't be a problem, honest.  The benefit is that a failed query is only
	 * going to affect a single query in a single version.  Otherwise if there
	 * were three table update querys and the third one failed, it would not be
	 * possible to fix only the third query and run the update again.  it would
	 * have to be run manually or moved to a new version block anyway.  The
	 * single update per version rule ensures that we only need to fix the one
	 * query and compile to continue schema updates.  It is acceptable, and
	 * encourages to group updates to each table in a single update if they are
	 * all being made at the same time. */
	/* }}} End notes */
	switch(schemaversion)
	{
		case 0:  /* {{{ New database - add config table */
		{
			cout << "New database detected, building from Schema 0" << endl;
			cout << "This may take a few minutes." << endl;
			/* Create the config table and insert the schema version record */
			{
				QString q;
				QTextOStream qos(&q);
				qos << "create table config (" << endl;
				qos << "vid int not null auto_increment," << endl;
				qos << "vname varchar(50) not null," << endl;
				qos << "vval varchar(255) not null," << endl;
				qos << "primary key (vid)," << endl;
				qos << "index idx_nm(vname));";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 1" << endl;
					cout << "Query: " << q << endl;
					return false;
				}
			}
			{
				QString q;
				QTextOStream qos(&q);
				qos << "insert into config (vname, vval) values" << endl;
				qos << "('SchemaVersion', '1');";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 1" << endl;
					cout << "Query: " << q << endl;
					return false;
				}
			}
		} /* }}} */
		case 1: /* {{{ db at version 1, add player table to upgrade to ver2 */
		{
			cout << "Database schema at version 1, upgrading to version 2" << endl;
			/* Create a basic player table - playerid, name, password, email */
			{
				QString q;
				QTextOStream qos(&q);
				qos << "create table players (" << endl;
				qos << "playerid int not null auto_increment," << endl;
				qos << "name varchar(30) not null," << endl;
				qos << "pass varchar(32) not null," << endl;
				qos << "email varchar(255)," << endl;
				qos << "created datetime not null," << endl;
				qos << "lastlogin datetime not null," << endl;
				qos << "primary key(playerid)," << endl;
				qos << "unique (name));";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 2" << endl;
					cout << "Query: " << q << endl;
					return false;
				}
			}
			{
				QString q;
				QTextOStream qos(&q);
				qos << "update config" << endl;
				qos << "set vval = '2'" << endl;
				qos << "where vname='SchemaVersion';";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 2" << endl;
					cout << "Query: " << q << endl;
					return false;
				}
			}
		} /* }}} */
		case 2:  /* {{{ Schema version 2 is current */
		{
			cout << "Database schema at version 2 and current" << endl;
		} /* }}} */
	}
	return true;
}

bool initdatabasesystem(void)
{
	QSqlDatabase *defaultDB = QSqlDatabase::addDatabase( SQLDRIVER );
	if ( defaultDB ) {
		defaultDB->setDatabaseName( SQLDB );
		defaultDB->setUserName( SQLUSER );
		defaultDB->setPassword( SQLPASS );
		defaultDB->setHostName( SQLHOST );

		if ( defaultDB->open() ) {
			/* Database is open.  Lets check the schema version and upgrade the
			 * database if needed */
			if (checkandupgradeschema())
				return true;
			else
				return false;
		}
	}
	return false;
}
