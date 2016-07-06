#ifndef MYSIGNAL_H
#define MYSIGNAL_H
#include <qpushbutton.h>

class MyEmitter :public QPushButton
{
    Q_OBJECT

public:
    MyEmitter( const QString& label, QWidget* parent = 0 );

signals:
    void aSignal();
    void anOtherSignal( int, QWidget* );
};

#endif /* MYSIGNAL_H */

