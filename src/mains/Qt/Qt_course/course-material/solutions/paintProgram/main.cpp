#include "paintwindow.h"
#include <QApplication>

int main( int argc, char* argv[] )
{
  QApplication myapp( argc, argv );

  PaintWindow* mywidget = new PaintWindow( 0 );
  mywidget->resize( 400, 400 );

  mywidget->show();
  return myapp.exec();
}
