// ==========================================================================
// Program TEXTEDITOR
// ==========================================================================
// Last updated on 7/24/07
// ==========================================================================

#include <iostream>
#include <QtGui>
#include "Editor.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   QApplication my_app(argc,argv);

   Editor* editor_ptr=new Editor;
   editor_ptr->show();

   return my_app.exec();
}

