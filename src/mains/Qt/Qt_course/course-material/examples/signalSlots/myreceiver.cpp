#include "myreceiver.h"

MyReceiver::MyReceiver( QObject* parent ) :QObject( parent )
{
}

void MyReceiver::aSlot()
{
    qDebug("A Slot");
}


void MyReceiver::anOtherSlot( int, QObject* )
{
    qDebug("anOtherSlot");
}


