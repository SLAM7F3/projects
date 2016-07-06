// ==========================================================================
// Program TEMPERATURE
// ==========================================================================
// Last updated on 7/25/07
// ==========================================================================

#include <iostream>
#include <QtGui>
#include "Thermometer.h"

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

   Thermometer* thermometer_ptr=new Thermometer;
   thermometer_ptr->show();

   return my_app.exec();
}

