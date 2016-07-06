-// ==========================================================================
// Program SLIDER
// ==========================================================================
// Last updated on 7/23/07
// ==========================================================================

#include <iostream>
#include <QtGui>

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

   QLabel* nameLabel_ptr=new QLabel("Username: ");
   QLineEdit* nameEdit_ptr=new QLineEdit;
  
   QLabel* passwordLabel_ptr=new QLabel("Password: ");
   QLineEdit* passwordEdit_ptr=new QLineEdit;
   
   QHBoxLayout* nameLayout_ptr=new QHBoxLayout;
   nameLayout_ptr->addWidget(nameLabel_ptr);
   nameLayout_ptr->addWidget(nameEdit_ptr);

   QHBoxLayout* passwordLayout_ptr=new QHBoxLayout;
   passwordLayout_ptr->addWidget(passwordLabel_ptr);
   passwordLayout_ptr->addWidget(passwordEdit_ptr);
   
   QVBoxLayout* windowLayout_ptr=new QVBoxLayout;
   windowLayout_ptr->addLayout(nameLayout_ptr);
   windowLayout_ptr->addLayout(passwordLayout_ptr);

   QWidget* window_ptr=new QWidget;
   window_ptr->setWindowTitle("Slider example:");
   window_ptr->setLayout(windowLayout_ptr);
   
   window_ptr->show();

   return app.exec();
}

