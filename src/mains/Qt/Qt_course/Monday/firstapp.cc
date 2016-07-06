// ==========================================================================
// Program FIRSTAPP
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

// Name and street section:

   QLabel* nameLabel_ptr=new QLabel("Name: ");
   QLineEdit* nameEdit_ptr=new QLineEdit;

   QHBoxLayout* nameLayout_ptr=new QHBoxLayout;
   nameLayout_ptr->addWidget(nameLabel_ptr);
   nameLayout_ptr->addWidget(nameEdit_ptr);

  
   QLabel* passwordLabel_ptr=new QLabel("Street: ");
   QLineEdit* passwordEdit_ptr=new QLineEdit;

   QHBoxLayout* passwordLayout_ptr=new QHBoxLayout;
   passwordLayout_ptr->addWidget(passwordLabel_ptr);
   passwordLayout_ptr->addWidget(passwordEdit_ptr);
   
// Country section:

   QLabel* CountryLabel_ptr=new QLabel("Country:");
   QListWidget* CountryListWidget_ptr=new QListWidget();
   CountryListWidget_ptr->addItem("England");
   CountryListWidget_ptr->addItem("Denmark");
   CountryListWidget_ptr->addItem("France");

   QVBoxLayout* CountryLayout_ptr=new QVBoxLayout;
   CountryLayout_ptr->addWidget(CountryLabel_ptr);
   CountryLayout_ptr->addWidget(CountryListWidget_ptr);

// Comments section:

   QLabel* CommentsLabel_ptr=new QLabel("Comments:");
   QVBoxLayout* CommentsLayout_ptr=new QVBoxLayout;
   CommentsLayout_ptr->addWidget(CommentsLabel_ptr);

// Overall layout for entire window:

   QVBoxLayout* windowLayout_ptr=new QVBoxLayout;
   windowLayout_ptr->addLayout(nameLayout_ptr);
   windowLayout_ptr->addLayout(passwordLayout_ptr);
   windowLayout_ptr->addLayout(CountryLayout_ptr);
   windowLayout_ptr->addLayout(CommentsLayout_ptr);

// Instantiate entire window:

   QWidget* window_ptr=new QWidget;
   window_ptr->setWindowTitle("First application:");
   window_ptr->setLayout(windowLayout_ptr);
   
   window_ptr->show();

   return app.exec();
}

