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
#include "logging.hxx"

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

/** Get listen ports from database
 * Return a value list with the ports we should listen on.
 * @note  Eventually we will need to add the type of listen port along with
 * the port number.
 */
QValueList<int> Database::getListenPorts(QString profile)
{
	QString query;
	QTextOStream qos(&query);
	QSqlQuery q;
	QValueList<int> pl;

	qos << "select vval from config" << endl
			<< "where vname like '" << profile << "-port%';";
	if (q.exec(query) && q.numRowsAffected() > 0)
	{
		while (q.next())
		{
			pl << q.value(0).toInt();
		}
	} else {
		pl << 9000;
	}

	return pl;
}

/** Validate and upgrade database schema
	 * 
	 * @note  There are *NO* break statements between cases.
	 * @note  This list *is* very order sensitive.  Changing the order of items
	 * *will* break schema upgrades
	 * @note  Since the overall schema version number really doesn't have an
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
	 * all being made at the same time.
	 */
void Database::checkschema(void)
{
	int schemaversion = 0;
	QSqlQuery query;

	/* Get a random value from the sql server to seed our PRNG */
	query.exec("select RAND();");
	query.next();
	srandom((unsigned int)(query.value(0).toDouble()*100000));

	QSqlQuery getschemaver("select vval from config where vname='SchemaVersion';");
	if (getschemaver.isActive())
	{
		getschemaver.next();
		schemaversion = getschemaver.value(0).toInt();
	}

	/* Each line in this case statement upgrades the schema to the next version.
	 * The last case takes no action as the latest version.
	 */
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
				schemaversion++;
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
				schemaversion++;
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
				qos << "zoneid int not null," << endl;
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
				schemaversion++;
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
				qos << "zone int not null," << endl;
				qos << "latitude int not null," << endl;
				qos << "longitude int not null," << endl;
				qos << "elevation int not null," << endl;
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
				qos << "primary key (zone,latitude,longitude,elevation)," << endl;
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
				schemaversion++;
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
				qos << "insert into zone values" << endl;
				qos << "(0,'Junk Zone', 'Default zone for miscelaneous junk'," << endl;
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
				schemaversion++;
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 5" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
		} /* }}} */
		case 5: /* {{{ db at version 5, Add empty room #1 */
		{
			cout << "Database schema at version 5, upgrading to version 6" << endl;
			{
				QString q;
				QTextOStream qos(&q);
				qos << "insert into room (zone,latitude, longitude, elevation," << endl;
				qos << "title, flags, description)" << endl;
				qos << "values" << endl;
				qos << "(0,0,0,0,'The Origin',";
				qos << "'safe,savespot,recallspot'," << endl;
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
				schemaversion++;
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 6" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
		} /* }}} */
		case 6: /* {{{ db at version 6, Add room info to player table */
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
				schemaversion++;
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
				schemaversion++;
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 8" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
		} /* }}} */
		case 8: /* {{{ db at version 8, Add commandgroup table */
		{
			cout << "Database schema at version 8, upgrading to version 9" << endl;
			{
				QString q;
				QTextOStream qos(&q);
				qos << "create table commandgroup (" << endl;
				qos << "gid int not null auto_increment primary key," << endl;
				qos << "gname varchar(50) not null unique" << endl;
				qos << ");";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 9" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
			{
				QString q;
				QTextOStream qos(&q);
				qos << "update config" << endl;
				qos << "set vval = '9'" << endl;
				qos << "where vname='SchemaVersion';";
				schemaversion++;
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 9" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
		} /* }}} */
		case 9: /* {{{ db at version 9, Add group membership table */
		{
			cout << "Database schema at version 9, upgrading to version 10" << endl;
			{
				QString q;
				QTextOStream qos(&q);
				qos << "create table groupmem (" << endl;
				qos << "playerid int not null," << endl;
				qos << "groupid int not null," << endl;
				qos << "primary key (playerid, groupid)," << endl;
				qos << "foreign key fk_pid (playerid) references players (playerid),"
						<< endl;
				qos << "foreign key fk_gid (groupid) references commandgroup (gid)"
						<< endl;
				qos << ");";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 10" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
			{
				QString q;
				QTextOStream qos(&q);
				qos << "update config" << endl;
				qos << "set vval = '10'" << endl;
				qos << "where vname='SchemaVersion';";
				schemaversion++;
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 10" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
		} /* }}} */
		case 10: /* {{{ db at version 10, Add command permissions table */
		{
			cout << "Database schema at version 10, upgrading to version 11" << endl;
			{
				QString q;
				QTextOStream qos(&q);
				qos << "create table cmdperm (" << endl;
				qos << "playerid int not null," << endl;
				qos << "cmdname varchar(25) not null," << endl;
				qos << "allowed enum('no', 'yes') not null default 'no'," << endl;
				qos << "primary key (playerid, cmdname)," << endl;
				qos << "foreign key fk_pid (playerid) references players (playerid)"
						<< endl;
				qos << ");";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 11" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
			{
				QString q;
				QTextOStream qos(&q);
				qos << "update config" << endl;
				qos << "set vval = '11'" << endl;
				qos << "where vname='SchemaVersion';";
				schemaversion++;
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 11" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
		} /* }}} */
		case 11: /* {{{ db at version 11, Add default command groups */
		{
			cout << "Database schema at version 11, upgrading to version 12" << endl;
			{
				QString q;
				QTextOStream qos(&q);
				qos << "insert into commandgroup (gname) values" << endl;
				qos << "('Implementor')," << endl;
				qos << "('Builder')," << endl;
				qos << "('Coder')," << endl;
				qos << "('Immortal');";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 12" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
			{
				QString q;
				QTextOStream qos(&q);
				qos << "update config" << endl;
				qos << "set vval = '12'" << endl;
				qos << "where vname='SchemaVersion';";
				schemaversion++;
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 12" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
		} /* }}} */
		case 12: /* {{{ db at version 12, Add help table */
		{
			cout << "Database schema at version 12, upgrading to version 13" << endl;
			{
				QString q;
				QTextOStream qos(&q);
				qos << "create table helptext (" << endl;
				qos << "helpid int not null auto_increment primary key," << endl;
				qos << "title varchar(80) not null," << endl;
				qos << "keywords varchar(254)," << endl;
				qos << "body text not null," << endl;
				qos << "index idx_kw (keywords)," << endl;
				qos << "fulltext (title, keywords, body)" << endl;
				qos << ");";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 13" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
			{
				QString q;
				QTextOStream qos(&q);
				qos << "update config" << endl;
				qos << "set vval = '13'" << endl;
				qos << "where vname='SchemaVersion';";
				schemaversion++;
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 13" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
		} /* }}} */
		case 13: /* {{{ db at version 13, Add last name to players table */
		{
			cout << "Database schema at version 13, upgrading to version 14"<< endl;
			{
				QString q;
				QTextOStream qos(&q);
				qos << "alter table players" << endl
						<< "add inroomzone int not null default 0 after lastlogin,"
						<< endl
						<< "add lastname varchar(40) after name," << endl
			<< "add foreign key room (inroomzone,inroomlat,inroomlong,inroomelev) "
						<< "references room (zone,longitude,latitude,elevation) "
						<< "on delete set default;";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 14" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
			{
				QString q;
				QTextOStream qos(&q);
				qos << "update config" << endl;
				qos << "set vval = '14'" << endl;
				qos << "where vname='SchemaVersion';";
				schemaversion++;
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version 14" << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
		} /* }}} */
		case 14: /* {{{ db at version 14, Add welcome art table */
		{
			cout << "Database schema at version " << schemaversion
					 << ", upgrading to version " << schemaversion+1 << endl;
			{
				QString q;
				QTextOStream qos(&q);
				qos << "create table welcomeart (" << endl
						<< "name varchar(30) not null primary key," << endl
						<< "art text not null);";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version "
							 << schemaversion+1 << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
			{
				QString q;
				QTextOStream qos(&q);
				qos << "update config" << endl;
				qos << "set vval = '" << ++schemaversion << "'" << endl;
				qos << "where vname='SchemaVersion';";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version "
							 << schemaversion << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
		} /* }}} */
		case 15: /* {{{ db at version 15, Add languages table */
		{
			cout << "Database schema at version " << schemaversion
					 << ", upgrading to version " << schemaversion+1 << endl;
			{
				QString q;
				QTextOStream qos(&q);
				qos << "create table languages (" << endl
						<< "langid char(5) not null primary key," << endl
						<< "name varchar(50) not null unique," << endl
						<< "parentid char(5) not null," << endl
						<< "charset varchar(50) not null," << endl
						<< "notes varchar(255)," << endl
						<< "difficulty smallint not null default 50," << endl
						<< "shortname varchar(20) not null);";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version "
							 << schemaversion+1 << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
			{
				QString q;
				QTextOStream qos(&q);
				qos << "update config" << endl;
				qos << "set vval = '" << ++schemaversion << "'" << endl;
				qos << "where vname='SchemaVersion';";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version "
							 << schemaversion << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
		} /* }}} */
		case 16: /* {{{ db at version 16, Add roomexits table */
		{
			cout << "Database schema at version " << schemaversion
					 << ", upgrading to version " << schemaversion+1 << endl;
			{
				QString q;
				QTextOStream qos(&q);
				qos << "create table roomexits (" << endl
						<< "r1zone int not null," << endl
						<< "r1lat int not null," << endl
						<< "r1long int not null," << endl
						<< "r1elev int not null," << endl
						<< "r2zone int not null," << endl
						<< "r2lat int not null," << endl
						<< "r2long int not null," << endl
						<< "r2elev int not null," << endl
						<< "name varchar(20)," << endl
						<< "keyobj int not null default 0," << endl
						<< "flags set('DOOR', 'LOCKABLE', 'LOCKED', 'PICKPROOF'," << endl
						<< "'CLOSED', 'MAGICLOCK', 'HIDDEN', 'TRAPPED'," << endl
						<< "'MAGICPROOF', 'BASHPROOF', 'TWOWAY')," << endl
						<< "direction varchar(10)," << endl
						<< "primary key (r1zone, r1lat, r1long, r1elev,"
						<< "r2zone, r2lat, r2long, r2elev, direction));";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version "
							 << schemaversion+1 << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
			{
				QString q;
				QTextOStream qos(&q);
				qos << "update config" << endl;
				qos << "set vval = '" << ++schemaversion << "'" << endl;
				qos << "where vname='SchemaVersion';";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version "
							 << schemaversion << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
		} /* }}} */
		case 17: /* {{{ db at version 17, Change room Flags column */
		{
			cout << "Database schema at version " << schemaversion
					 << ", upgrading to version " << schemaversion+1 << endl;
			{
				QString q;
				QTextOStream qos(&q);
				qos << "alter table room" << endl
						<< "change flags "
						<< "flags set('levelrest','regen','deathtrap',"
						<< "'nonpc','safe','savespot','recallspot','notrack',"
						<< "'nomagic','noteleport', 'dizzy') default '' not null," << endl
						<< "add lightlev smallint not null default 50"
						<< ";";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version "
							 << schemaversion+1 << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
			{
				QString q;
				QTextOStream qos(&q);
				qos << "update config" << endl;
				qos << "set vval = '" << ++schemaversion << "'" << endl;
				qos << "where vname='SchemaVersion';";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version "
							 << schemaversion << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
		} /* }}} */
		case 18: /* {{{ db at version 18, Add skilllevels table */
		{
			cout << "Database schema at version " << schemaversion
					 << ", upgrading to version " << schemaversion+1 << endl;
			{
				QString q;
				QTextOStream qos(&q);
				qos << "create table skilllevels (" << endl
						<< "pid int not null," << endl
						<< "skid char(5) not null," << endl
						<< "learned tinyint not null default 1," << endl
						<< "primary key (pid, skid)," << endl
						<< "foreign key fk_pid (pid) references players (playerid) "
						<< "match full on delete cascade );";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version "
							 << schemaversion+1 << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
			{
				QString q;
				QTextOStream qos(&q);
				qos << "update config" << endl;
				qos << "set vval = '" << ++schemaversion << "'" << endl;
				qos << "where vname='SchemaVersion';";
				if (!query.exec(q))
				{
					cout << "FATAL: error upgrading schema to version "
							 << schemaversion << endl;
					cout << "Query: " << q << endl;
					return;
				}
			}
		} /* }}} */
		default:  /* {{{ Schema version is current */
		{
			cout << "Database schema at version " << schemaversion 
					 << " and current." << endl;
		} /* }}} */
	}
	dbonline = true;
}

}; /** end koalamud namespace */
