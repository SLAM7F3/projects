// ==========================================================================
// Program SLIDER 
// ==========================================================================
// Last updated on 7/23/07
// ==========================================================================

#include <iostream>
#include <QtGui>
#include "InOut.h"

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

   QWidget* window_ptr=new QWidget;
   window_ptr->setWindowTitle("2nd slider example:");

   InOut* inout_ptr=new InOut(window_ptr);

   window_ptr->setLayout(inout_ptr->get_layout_ptr());
   window_ptr->show();

   return app.exec();
}

