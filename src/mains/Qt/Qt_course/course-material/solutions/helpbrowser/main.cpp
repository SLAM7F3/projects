#include<QtGui>
#include "test-widget.h"

int main( int argc, char** argv)
{
  QApplication application( argc, argv );

  TestWidget* test = new TestWidget( 0 );
  test->show();

  return application.exec();
}
