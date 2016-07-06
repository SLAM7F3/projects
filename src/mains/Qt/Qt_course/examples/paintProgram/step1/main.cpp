#include <QApplication>
#include "scribbleArea.h"

int main( int argc, char* argv[] )
{
  QApplication myapp( argc, argv );

  ScribbleArea* mywidget = new ScribbleArea( 0 );
  mywidget->resize( 400, 400 );

  mywidget->show();
  return myapp.exec();
}
