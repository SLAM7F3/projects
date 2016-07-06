// ==========================================================================
// Header file for gotocelldialog class
// ==========================================================================
// Last modified on 7/26/07
// ==========================================================================

#ifndef GOTOCELLDIALOG_H
#define GOTOCELLDIALOG_H

#include <QDialog>
#include "ui_gotocelldialog.h"

class GoToCellDialog : public QDialog, private Ui::GoToCellDialog
{

   Q_OBJECT

      public:
   
   GoToCellDialog(QWidget* parent=NULL);
   
   
   private slots:

      void on_lineEdit_textChanged();

};

#endif  // gotocelldialog.h
