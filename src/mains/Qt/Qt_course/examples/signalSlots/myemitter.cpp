#include "myemitter.h"

MyEmitter::MyEmitter( const QString& label, QWidget* parent ) :QPushButton( label, parent )
{
    connect( this, SIGNAL(clicked()), this, SIGNAL(aSignal()));
}


