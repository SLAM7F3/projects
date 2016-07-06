#include <QtGui>
#include "listener.h"

int main(int argc, char** argv)
{
    QApplication app( argc, argv );
    Listener* listener = new Listener(0);
    listener->show();
    return app.exec();
}


