#include <QtGui>
#include "findDialog.h"

int main( int argc, char** argv )
{
  QApplication app( argc, argv );
  FindDialog* dialog = new FindDialog( 0 );
  return dialog->exec();
}



