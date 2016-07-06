// ==========================================================================
// Program DESIGNER_DIALOG
// ==========================================================================
// Last updated on 7/26/07
// ==========================================================================

#include <iostream>
#include <QApplication>
#include <QDialog>
#include "gotocelldialog.h"

using std::cin;
using std::cout;
using std::endl;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   QApplication app(argc,argv);

   GoToCellDialog* dialog_ptr=new GoToCellDialog;
   dialog_ptr->show();
   
   return app.exec();
}

