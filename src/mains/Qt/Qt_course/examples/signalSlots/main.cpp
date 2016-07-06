#include <QtGui>
#include "myemitter.h"
#include "myreceiver.h"

int main( int argc, char** argv )
{
    QApplication app( argc, argv );

    MyEmitter* sig = new MyEmitter( "Hello World"  );
    MyReceiver* slot = new MyReceiver;
    QObject::connect( sig, SIGNAL(aSignal()), slot, SLOT(aSlot()));

    sig->show();

    return app.exec();
}
