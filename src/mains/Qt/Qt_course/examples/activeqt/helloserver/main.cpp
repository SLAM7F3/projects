#include <QtGui>
#include <QAxFactory>
#include "hello.h"

QAXFACTORY_BEGIN("{B3D41CE9-EF63-474f-9A1B-CC243C1266F5}",
                 "{55B44106-7381-4e46-B47F-30D19BE49D0C}" )
  QAXCLASS(Hello)
QAXFACTORY_END()

int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    if ( !QAxFactory::isServer() ) {
       // create and show main window...
       Hello* hello = new Hello(0);
       hello->show();
    }

    return app.exec();
}
