#ifndef FLOATINGFRAMEDIALOG_H
#define FLOATINGFRAMEDIALOG_H

#include "ui_floatingframe.h"
#include <QTextLength>

class FloatingFrameDialog : public QDialog, private Ui::FloatingFrame {
    Q_OBJECT

public:
    FloatingFrameDialog( QWidget* parent );
    QTextLength length();
    QTextFrameFormat::Position position();

protected slots:
    void typeChanged( int );
};


#endif /* FLOATINGFRAMEDIALOG_H */

