#ifndef REPLACEPROGRESSDIALOG_H
#define REPLACEPROGRESSDIALOG_H

#include "ui_replaceprogress.h"
#include <QTextCursor>
class QTextEdit;

class ReplaceProgressDialog :public QDialog, private Ui::ReplaceProgress
{
    Q_OBJECT

public:
    ReplaceProgressDialog( QTextEdit* edit, const QString& search, const QString& replace, QWidget* parent );

protected slots:
    void replaceCurrent();
    void skipCurrent();
    void replaceAll();

protected:
    void findNext();

private:
    QTextEdit* _edit;
    QTextCursor _cursor;
    QString _search;
    QString _replacement;
    bool _replaceAll;
};


#endif /* REPLACEPROGRESSDIALOG_H */

