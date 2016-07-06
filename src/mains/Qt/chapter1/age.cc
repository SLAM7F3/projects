// ==========================================================================
// Program AGE
// ==========================================================================
// Last updated on 7/10/07
// ==========================================================================

#include <iostream>
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>

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
   window_ptr->setWindowTitle("Enter your age:");
   
   window_ptr->show();

   return app.exec();
}

