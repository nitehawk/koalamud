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

#ifndef KOALA_PLAYERCHAR_HXX
#define KOALA_PLAYERCHAR_HXX "%A%"

#include <qobject.h>
#include <qlistview.h>
#include <qptrlist.h>
#include <qptrqueue.h>

#include <zthread/FastRecursiveMutex.h>
#include <zthread/Guard.h>
#include <zthread/Thread.h>

#include "main.hxx"
#include "autoptr.hxx"
#include "char.hxx"
#include "network.hxx"
#include "koalastatus.h"

class K_PlayerChar;
#include "cmd.hxx"
#include "comm.hxx"

/* Small struct used to hold the strings for commands so they can be queued in
 * a text-in queue */
typedef struct {
	QString line;
	QString cline;
	QString cmdword;
} plrtextin_t;
typedef plrtextin_t *plrtextin_pt;

/* This task class is used to run commands in one of the threadpool threads to
 * make sure we don't block the entire system executing commands */
class K_PCInputTask : public ZThread::Runnable
{
	protected:
		KRefPtr<K_PlayerChar> ch;

	public:
		K_PCInputTask(K_PlayerChar *plrchar);
		K_PCInputTask(KRefPtr<K_PlayerChar> &plrchar);
		virtual ~K_PCInputTask(void);
		virtual void run(void) throw();

};


/**
 * Player Character object - Handles interaction that is specific to player characters.
 * Matthew Schlegel
 **/
class K_PlayerChar : public virtual KoalaDescriptor, public virtual K_Char,
										 public virtual KRefObj
{
    Q_OBJECT

		public:
		typedef enum {STATE_GETNAME, STATE_NEWPLAYER, STATE_NEWPLAYERCONFIRM,
									STATE_NEWPASS, STATE_NEWPASSCONFIRM, STATE_GETEMAIL,
									STATE_EMAILCONFIRM, STATE_GETPASS, STATE_PLAYING} state_t;

		protected:
		state_t _state;
		QString _password;
		QString _email;  // Optional player field
    
    public:
    K_PlayerChar(int sock, QObject *parent=0, const char *name=0);
    virtual ~K_PlayerChar();
		virtual void setName(QString name);
		virtual void sendWelcome(void);
		virtual bool event(QEvent *e);

		/** Operator new overload */
		void * operator new(size_t obj_size)
			{ return poolalloc.alloc(obj_size); }

		/** Operator delete overload */
		void operator delete(void *ptr)
			{ poolalloc.free(ptr); }
    
    private slots:
    virtual void readclient(void);

		public slots:
		virtual void setdisconnect(bool dis = true) { _disconnecting = dis; }
		virtual void runcmd(cmdentry_t *cmd, QString word, QString arg);
		virtual void parseline(QString line, QString cline, QString cmdword);
		virtual void updateguistatus(void);
		virtual bool sendtochar(QString text);

		virtual void channelsendtochar(K_PlayerChar *, QString, QString, QString);
		virtual void channeldeleted(KoalaChannel *);
		virtual void save(void);
		virtual bool load(bool checkpass = true);
    
    protected:
    QListViewItem *plrstatuslistitem;
		bool _disconnecting;
		bool _indatabase;

		QPtrQueue<plrtextin_t> lineinqueue;
		ZThread::FastRecursiveMutex linequeuelock;
		// If true, there is either a command task running or scheduled and
		// readclient doesn't need to create one.
		bool cmdtaskrunning;  

		friend class K_PCInputTask;
};

/* Two lists of players used, one linked list and one hashmap */
typedef QPtrList<K_PlayerChar> playerlist_t;
typedef QPtrListIterator<K_PlayerChar> playerlistiterator_t;

#ifdef KOALA_PLAYERCHAR_CXX
playerlist_t connectedplayerlist;
QDict<K_PlayerChar> connectedplayermap(101, false);
#else
extern playerlist_t connectedplayerlist;
extern QDict<K_PlayerChar> connectedplayermap;
#endif

#endif  //  KOALA_PLAYERCHAR_HXX
