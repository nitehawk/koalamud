/****************************************************************************
** Form interface generated from reading ui file 'src/koalastatus.ui'
**
** Created: Fri Aug 23 22:36:50 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef KOALASTATUS_H
#define KOALASTATUS_H

#include <qvariant.h>
#include <qmainwindow.h>
#include <qdockwindow.h>
#include <qlistview.h>

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QAction;
class QActionGroup;
class QToolBar;
class QPopupMenu;

class KoalaStatus : public QMainWindow
{ 
    Q_OBJECT

public:
    KoalaStatus( QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel );
    ~KoalaStatus();

    QMenuBar *menubar;
    QPopupMenu *System;
    QPopupMenu *Status;
    QAction* Quit;
    QAction* StartNetwork;
    QAction* PlayerDockAct;
    QAction* ListenDockAct;
    QDockWindow *PlayerStatusDock;
    QDockWindow *ListenStatusDock;
    QListView *PlayerStatusList;
    QListView *ListenStatusList;
    QListViewItem *PlayerCountItem;
    QListViewItem *ListenCountItem;

public slots:
    virtual void playerstoggled(bool);
    virtual void listentoggled(bool);
    virtual void portdialog( void );
    void updateplayercount(void);
    void updatelistencount(void);

};

#endif // KOALASTATUS_H
