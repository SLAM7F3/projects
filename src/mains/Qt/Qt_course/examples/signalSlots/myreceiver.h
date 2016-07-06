#ifndef MYSLOT_H
#define MYSLOT_H
#include <qobject.h>

class MyReceiver :public QObject
{
    Q_OBJECT

public:
    MyReceiver( QObject* parent = 0 );

public slots:
    void aSlot();
    void anOtherSlot( int, QObject* );
};


#endif /* MYSLOT_H */

