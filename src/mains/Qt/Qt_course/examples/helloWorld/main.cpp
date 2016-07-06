#include <QtGui>

int main( int argc, char* argv[] )
{
  QApplication myapp( argc, argv );
  QLabel* mylabel = new QLabel( "Hello world", 0 );
  mylabel->show();
  return myapp.exec();
}
