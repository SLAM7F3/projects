#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include <QString>
#include "ui_findDialog.h"

class QProcess;

class FindDialog :public QDialog, private Ui::FindDialog
{
    Q_OBJECT

public:
    FindDialog( QWidget* parent );

protected slots:
    void on_editButton_clicked();
    void on_goButton_clicked();
    void on_quitButton_clicked();

    void endOfProcess();
    void readStdout();
    void readStderr();
    void slotEndProcess();

private:
    QProcess* _process;
};


#endif /* FINDDIALOG_H */

