#include "colorTester.h"
#include <QApplication>

int main( int argc, char** argv )
{
    QApplication app( argc, argv );

    ColorTester* tester = new ColorTester( 0 );
    tester->show();

    return app.exec();
}
