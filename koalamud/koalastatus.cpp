/***************************************************************\
*                        KoalaMud Gen 2                         *
*    Copyright (c) 2002 First Step Internet Services, Inc.      *
*                     All Rights Reserved                       *
*        Distributed under the terms of the FSI License         *
* See file LICENSE in the root of this package for information  *
\***************************************************************/
/***************************************************************\
*	Module: GUI
* Description: Main GUI display window
* Classes:  KoalaStatus
\***************************************************************/

#define KOALA_KOALASTATUS_CXX "%A%"

#include <qvariant.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>

#include "koalastatus.h"
#include "newnetworkportdlg.h"
/* 
 *  Constructs a KoalaStatus which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 */
KoalaStatus::KoalaStatus( QWidget* parent,  const char* name, WFlags fl )
    : QMainWindow( parent, name, fl )
{
	(void)statusBar();
	if ( !name )
		setName( "KoalaStatus" );
	resize( 600, 480 ); 
	setCaption( trUtf8( "KoalaMUD Status" ) );

	// actions
	Quit = new QAction( this, "Quit" );
	Quit->setText( trUtf8( "Quit" ) );
	StartNetwork = new QAction( this, "StartNetwork" );
	StartNetwork->setText( trUtf8( "StartNetwork" ) );
	PlayerDockAct = new QAction( this, "Players" );
	PlayerDockAct->setToggleAction(true);
	PlayerDockAct->setOn(true);
	PlayerDockAct->setText( trUtf8( "Players" ) );
	ListenDockAct = new QAction( this, "Listens" );
	ListenDockAct->setToggleAction(true);
	ListenDockAct->setOn(true);
	ListenDockAct->setText( trUtf8( "Listens" ) );


	// toolbars


	// menubar
	menubar = new QMenuBar( this, "menubar" );

	System = new QPopupMenu( this ); 
	StartNetwork->addTo( System );
	System->insertSeparator();
	Quit->addTo( System );
	menubar->insertItem( trUtf8( "System" ), System );

	Status = new QPopupMenu( this ); 
	PlayerDockAct->addTo( Status );
	ListenDockAct->addTo( Status );
	menubar->insertItem( trUtf8( "Status" ), Status );

	// Dock Windows
	PlayerStatusDock = new QDockWindow(QDockWindow::InDock, this,
						"PlrStat");
	PlayerStatusList = new QListView(PlayerStatusDock, "PlrStatList");
	PlayerStatusList->addColumn("Player");
	PlayerStatusList->addColumn("Level");
	PlayerCountItem = new QListViewItem(PlayerStatusList, "Total", "0");
	PlayerStatusDock->setWidget(PlayerStatusList);
	PlayerStatusDock->setResizeEnabled(true);
	moveDockWindow(PlayerStatusDock, DockLeft);
	setDockEnabled(PlayerStatusDock, DockTop, false);
	setDockEnabled(PlayerStatusDock, DockBottom, false);
	PlayerStatusDock->show();

	ListenStatusDock = new QDockWindow(QDockWindow::InDock, this,
						"ListenStat");
	ListenStatusList = new QListView(ListenStatusDock, "ListenStatList");
	ListenStatusList->addColumn("Listen Port");
	ListenStatusList->addColumn("Listener Status");
	ListenCountItem = new QListViewItem(ListenStatusList, "Total Listeners",
						"0");
	ListenStatusDock->setWidget(ListenStatusList);
	ListenStatusDock->setResizeEnabled(true);
	moveDockWindow(ListenStatusDock, DockLeft);
	setDockEnabled(ListenStatusDock, DockTop, false);
	setDockEnabled(ListenStatusDock, DockBottom, false);
	ListenStatusDock->show();

	// signals and slots connections
	connect( Quit, SIGNAL( activated() ), this, SLOT( close() ) );
	connect(StartNetwork, SIGNAL( activated() ), this, SLOT( portdialog() ) );
	connect(PlayerDockAct, SIGNAL(toggled(bool)), this, SLOT(playerstoggled(bool)));
	connect(ListenDockAct, SIGNAL(toggled(bool)), this, SLOT(listentoggled(bool)));
}

/*  
 *  Destroys the object and frees any allocated resources
 */
KoalaStatus::~KoalaStatus()
{
	/* Qt handles deleting all of the child objects for us */
}

void KoalaStatus::portdialog(void)
{
	NewNetworkPortDlg getport;
	getport.exec();
}

void KoalaStatus::listentoggled( bool status )
{
	/* Show the ListenStatus dockwin if status is true */
	if (status)
		ListenStatusDock->show();
	else
		ListenStatusDock->hide();

}

void KoalaStatus::playerstoggled( bool status )
{
	/* Show the PlayerStatus dockwin if status is true */
	if (status)
		PlayerStatusDock->show();
	else
		PlayerStatusDock->hide();
}

void KoalaStatus::updateplayercount(void)
{
    QString num;
    num.setNum(PlayerStatusList->childCount() - 1);
    PlayerCountItem->setText(1, num);
}

void KoalaStatus::updatelistencount(void)
{
    QString num;
    num.setNum(ListenStatusList->childCount() - 1);
    ListenCountItem->setText(1, num);     
}