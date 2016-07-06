// ==========================================================================
// Header file for finddialog class
// ==========================================================================
// Last modified on 7/10/07
// ==========================================================================

#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;

class FindDialog : public QDialog
{

   Q_OBJECT

      public:
   
   FindDialog(QWidget* parent=NULL);
   
  signals:

   void findNext(const QString& str, Qt::CaseSensitivity cs);
   void findPrevious(const QString& str, Qt::CaseSensitivity cs);

  private slots:

   void findClicked();
   void enableFindButton(const QString& text);

  private:

   QLabel* label_ptr;
   QLineEdit* lineEdit_ptr;
   QCheckBox* caseCheckBox_ptr;
   QCheckBox* backwardCheckBox_ptr;
   QPushButton* findButton_ptr;
   QPushButton* closeButton_ptr;
};

#endif  // finddialog.h
