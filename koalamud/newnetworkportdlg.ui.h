/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include "network.hxx"

/** Open a new listener port */
void NewNetworkPortDlg::openPort()
{
  /* We need to open a new port here - Server Class will take care of putting itself
   * the listener list and making sure the same port doesn't get opened multiple
   * times, etc. */
  new koalamud::Listener(newport->value());   

  done(Accepted);
}
