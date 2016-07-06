// ==========================================================================
// Program DIALOG
// ==========================================================================
// Last updated on 7/10/07
// ==========================================================================

#include <iostream>
#include <QApplication>
#include "finddialog.h"

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

   FindDialog* dialog_ptr=new FindDialog;
   dialog_ptr->show();

   return app.exec();
}

