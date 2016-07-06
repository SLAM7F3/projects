// =========================================================================
// FindDialog class member function definitions
// =========================================================================
// Last modified on 7/10/07
// =========================================================================

#include <iostream>
#include <QtGui>
#include "finddialog.h"

using std::cout;
using std::endl;
using std::ostream;
using std::vector;

// ---------------------------------------------------------------------
FindDialog::FindDialog(QWidget* parent):
   QDialog(parent)
{
   label_ptr=new QLabel(tr("Find &what:"));
   lineEdit_ptr=new QLineEdit;
   label_ptr->setBuddy(lineEdit_ptr);
   
   caseCheckBox_ptr=new QCheckBox(tr("Match &case"));
   backwardCheckBox_ptr=new QCheckBox(tr("Search &backward"));
   
   findButton_ptr=new QPushButton(tr("&Find"));
   findButton_ptr->setDefault(true);
   findButton_ptr->setEnabled(false);
   
   closeButton_ptr=new QPushButton(tr("Close"));
   
   connect(lineEdit_ptr, SIGNAL(textChanged(const QString&)),
           this, SLOT(enableFindButton(const QString &)));
   connect(findButton_ptr, SIGNAL(clicked()),
           this, SLOT(findClicked()));
   connect(closeButton_ptr, SIGNAL(clicked()),
           this, SLOT(close()));
   
   QHBoxLayout* topLeftLayout_ptr=new QHBoxLayout;
   topLeftLayout_ptr->addWidget(label_ptr);
   topLeftLayout_ptr->addWidget(lineEdit_ptr);
   
   QVBoxLayout* leftLayout_ptr=new QVBoxLayout;
   leftLayout_ptr->addLayout(topLeftLayout_ptr);
   leftLayout_ptr->addWidget(caseCheckBox_ptr);
   leftLayout_ptr->addWidget(backwardCheckBox_ptr);

   QVBoxLayout* rightLayout_ptr=new QVBoxLayout;
   rightLayout_ptr->addWidget(findButton_ptr);
   rightLayout_ptr->addWidget(closeButton_ptr);
   rightLayout_ptr->addStretch();

   QHBoxLayout* mainLayout_ptr=new QHBoxLayout;
   mainLayout_ptr->addLayout(leftLayout_ptr);
   mainLayout_ptr->addLayout(rightLayout_ptr);
   setLayout(mainLayout_ptr);

   setWindowTitle(tr("Find"));
   setFixedHeight(sizeHint().height());

}

void FindDialog::findClicked()
{
   QString text=lineEdit_ptr->text();
   Qt::CaseSensitivity cs=caseCheckBox_ptr->isChecked() ?
      Qt::CaseSensitive : Qt::CaseInsensitive;
   
   if (backwardCheckBox_ptr->isChecked())
   {
      emit findPrevious(text,cs);
   }
   else
   {
      emit findNext(text,cs);
   }
}

void FindDialog::enableFindButton(const QString& text)
{
   findButton_ptr->setEnabled(!text.isEmpty());
}
