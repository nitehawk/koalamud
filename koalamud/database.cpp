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

#include "database.hxx"

namespace koalamud {

/** Setup connection to database server
 * This function opens a connection to the specified database server with the
 * specified options.  It also calls the schema checking function to make sure
 * our database tables are up to date.
 * @param user Database user for logging into DB server
 * @param pass Database login password
 * @param db Database name
 * @param server Database server address
 * @param driver Qt Database driver - QMYSQL3 is the only tested driver
 */
Database::Database(QString user="koalamud", QString pass="k23hjdsav",
							 QString db="koalamud", QString server="localhost",
							 QString driver="QMYSQL3")
			: dbonline(false), defaultDB(NULL)
{
	defaultDB = QSqlDatabase::addDatabase( driver );
	if (!defaultDB)
		return;

	defaultDB->setDatabaseName( db );
	defaultDB->setUserName( user );
	defaultDB->setPassword( pass );
	defaultDB->setHostName( server );

	if ( !defaultDB->open() )
		return;

	checkschema();
}

/** Shutdown database connections */
Database::~Database(void)
{
	defaultDB->close();
}

/** Validate and upgrade database schema */
void Database::checkschema(void)
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
					return;
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
					return;
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
					return;
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
					return;
				}
			}
		} /* }}} */
		case 2: /* {{{ db at version 2, Add zone table */
		{
			cout << "Database schema at version 2, upgrading to version 3" << endl;
			{
				QString q;
				QTextOStream qos(&q);
				qos << "create table zone (" << endl;
				qos << "zoneid int not null auto_increment," << endl;
				qos << "name varchar(30) not null," << endl;
				qos << "description varchar(255)," << endl;
				qos << "status set('Online','OLC') default 'Online' not null," << endl;
				qos << "primary key (zoneid));";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 3" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
			{
				QString q;
				QTextOStream qos(&q);
				qos << "update config" << endl;
				qos << "set vval = '3'" << endl;
				qos << "where vname='SchemaVersion';";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 3" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
		} /* }}} */
		case 3: /* {{{ db at version 3, Add rooms table */
		{
			cout << "Database schema at version 3, upgrading to version 4" << endl;
			{
				QString q;
				QTextOStream qos(&q);
				qos << "create table room (" << endl;
				qos << "latitude int not null," << endl;
				qos << "longitude int not null," << endl;
				qos << "elevation int not null," << endl;
				qos << "zone int not null default 1," << endl;
				qos << "title varchar(50) not null," << endl;
				qos << "flags set('levelrest','regen','dark','deathtrap',";
				qos << "'nomob','safe','savespot','recallspot','notrack',";
				qos << "'nomagic','teleport') default '' not null," << endl;
				qos << "type enum('indoors', 'covered', 'field',";
				qos << "'city', 'farm', 'forest', 'hill', 'mountain',";
				qos << "'water', 'underwater', 'flying') " << endl;
				qos << "default 'field' not null," << endl;
				qos << "plrlimit int not null default 0," << endl;
				qos << "description text," << endl;
				qos << "primary key (latitude,longitude,elevation)," << endl;
				qos << "foreign key zonelink (zone) references zone (zoneid)" << endl;
				qos << " on delete set default" << endl;
				qos << ");";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 4" << endl;
					cout << "Error: " << query.lastError().databaseText() << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
			{
				QString q;
				QTextOStream qos(&q);
				qos << "update config" << endl;
				qos << "set vval = '4'" << endl;
				qos << "where vname='SchemaVersion';";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 4" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
		} /* }}} */
		case 4: /* {{{ db at version 4, Add zone #1 */
		{
			cout << "Database schema at version 4, upgrading to version 5" << endl;
			{
				QString q;
				QTextOStream qos(&q);
				qos << "insert into zone (name, description, status) values" << endl;
				qos << "('Junk Zone', 'Default zone for miscelaneous junk'," << endl;
				qos << "'Online,OLC');";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 5" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
			{
				QString q;
				QTextOStream qos(&q);
				qos << "update config" << endl;
				qos << "set vval = '5'" << endl;
				qos << "where vname='SchemaVersion';";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 5" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
		} /* }}} */
		case 5: /* {{{ db at version 5, Add empty root #1 */
		{
			cout << "Database schema at version 5, upgrading to version 6" << endl;
			{
				QString q;
				QTextOStream qos(&q);
				qos << "insert into room (latitude, longitude, elevation," << endl;
				qos << "title, zone, flags, description)" << endl;
				qos << "values" << endl;
				qos << "(0,0,0,'The Origin',";
				qos << "1, 'safe,savespot,recallspot'," << endl;
				qos << "'This empty room is the origin of the world');";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 6" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
			{
				QString q;
				QTextOStream qos(&q);
				qos << "update config" << endl;
				qos << "set vval = '6'" << endl;
				qos << "where vname='SchemaVersion';";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 6" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
		} /* }}} */
		case 6: /* {{{ db at version 5, Add room info to player table */
		{
			cout << "Database schema at version 6, upgrading to version 7" << endl;
			{
				QString q;
				QTextOStream qos(&q);
				qos << "alter table players" << endl;
				qos << "add inroomlat int not null default 0," << endl;
				qos << "add inroomlong int not null default 0," << endl;
				qos << "add inroomelev int not null default 0," << endl;
				qos << "add foreign key room (inroomlat,inroomlong,inroomelev) ";
				qos << "references room (longitude,latitude,elevation) ";
				qos << "on delete set default;";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 7" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
			{
				QString q;
				QTextOStream qos(&q);
				qos << "update config" << endl;
				qos << "set vval = '7'" << endl;
				qos << "where vname='SchemaVersion';";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 7" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
		} /* }}} */
		case 7: /* {{{ db at version 7, Add logging table */
		{
			cout << "Database schema at version 7, upgrading to version 8" << endl;
			{
				QString q;
				QTextOStream qos(&q);
				qos << "create table logging (" << endl;
				qos << "lid int not null auto_increment primary key," << endl;
				qos << "severity enum ('Fatal', 'Severe', 'Critical', ";
				qos << "'Error', 'Warning', 'Notice', 'Info', 'Debug') not null ";
				qos << " default 'Info'," << endl;
				qos << "profile varchar(100) not null default 'Default'," << endl;
				qos << "msgtime datetime not null," << endl;
				qos << "message text" << endl;
				qos << ");";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 8" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
			{
				QString q;
				QTextOStream qos(&q);
				qos << "update config" << endl;
				qos << "set vval = '8'" << endl;
				qos << "where vname='SchemaVersion';";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 8" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
		} /* }}} */
		case 8:  /* {{{ Schema version 8 is current */
		{
			cout << "Database schema at version 8 and current" << endl;
		} /* }}} */
	}
	dbonline = true;
}

}; /** end koalamud namespace */
