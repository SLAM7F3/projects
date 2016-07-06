// ==========================================================================
// Program HELLO
// ==========================================================================
// Last updated on 7/10/07
// ==========================================================================

#include <iostream>
#include <QApplication>
#include <QLabel>

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   QApplication app(argc,argv);
//   QLabel* label_ptr=new QLabel("Hello Qt!");
   QLabel* label_ptr=new QLabel(
      "<h2><i>Hello</i> "
      "<font color=red>Qt! </font></h2>");
//      "Hello Qt!");
   label_ptr->show();
   return app.exec();
}

