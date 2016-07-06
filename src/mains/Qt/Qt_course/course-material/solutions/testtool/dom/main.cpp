#include <QtGui>
#include "testtool.h"

int main(int argc, char** argv)
{
  QApplication a(argc,argv);
  TestTool* tt = new TestTool;
  tt->resize(400,400);
  tt->show();

  return a.exec();
}



