// =========================================================================
// GoToCellDialog class member function definitions
// =========================================================================
// Last modified on 7/26/07
// =========================================================================

#include <iostream>
#include <QtGui>
#include "gotocelldialog.h"

using std::cout;
using std::endl;

// ---------------------------------------------------------------------
GoToCellDialog::GoToCellDialog(QWidget* parent):
   QDialog(parent)
{
   setupUi(this);
   
   QRegExp regExp("[A-Za-z][1-9][0-9]{0,2}");
   lineEdit->setValidator(new QRegExpValidator(regExp,this));
   
   connect(okButton,SIGNAL(clicked()),
           this,SLOT(accept()));
   connect(cancelButton,SIGNAL(clicked()),
           this,SLOT(reject()));
}

void GoToCellDialog::on_lineEdit_textChanged()
{
   okButton->setEnabled(lineEdit->hasAcceptableInput());
}
