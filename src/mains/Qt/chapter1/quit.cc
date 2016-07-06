// ==========================================================================
// Program HELLO
// ==========================================================================
// Last updated on 7/10/07
// ==========================================================================

#include <iostream>
#include <QApplication>
#include <QLabel>
#include <QPushButton>

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
/*
   QLabel* label_ptr=new QLabel(
      "<h2><i>Hello</i> "
      "<font color=red>Qt! </font></h2>");
//      "Hello Qt!");
   label_ptr->show();
*/

   QPushButton* button_ptr=new QPushButton("Quit");
   QObject::connect(button_ptr,SIGNAL(clicked()),
                    &app, SLOT(quit()));
   button_ptr->show();
   return app.exec();
}

