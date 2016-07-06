#include <QtGui>
#include "server.h"

int main(int argc, char** argv)
{
  QApplication app( argc, argv );
  new Server;

  // Provide a button for quitting the server process; especially useful on Windows.
  QPushButton* quitButton = new QPushButton( "Quit server", 0 );
  quitButton->show();
  QObject::connect( quitButton, SIGNAL( clicked() ), qApp, SLOT( quit() ) );

  // If we didn't have a button then we would need to tell QApplication:
  // "don't quit after closing the first window":
  //qApp->setQuitOnLastWindowClosed( false );

  return app.exec();
}
