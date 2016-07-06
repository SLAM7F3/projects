#include "compass.h"
#include "compass2.h"
#include <QtGui>

int main( int argc, char* argv[] )
{
    QApplication app( argc, argv );

    QWidget window;

    CompassWidget* cw = new CompassWidget( &window );
    cw->setObjectName( "compass1" );
    CompassWidget2* cw2 = new CompassWidget2( &window );
    cw->setObjectName( "compass2" );
    QLabel* label = new QLabel( "North", &window );
    label->setObjectName( "label" );
    QObject::connect( cw, SIGNAL( directionChanged( const QString& ) ),
                      label, SLOT( setText( const QString& ) ) );
    QObject::connect( cw2, SIGNAL( directionChanged( const QString& ) ),
                      label, SLOT( setText( const QString& ) ) );
    QObject::connect( cw, SIGNAL( directionChanged( CompassDirection ) ),
                      cw2, SLOT( setDirection( CompassDirection ) ) );
    QObject::connect( cw2, SIGNAL( directionChanged( CompassDirection ) ),
                      cw, SLOT( setDirection( CompassDirection ) ) );
    cw2->setFocus();

    QVBoxLayout* vbl = new QVBoxLayout( &window );
    vbl->addWidget( cw, 2 );
    vbl->addWidget( cw2, 2 );
    vbl->addWidget( label );

    window.show();

    return app.exec();
}

