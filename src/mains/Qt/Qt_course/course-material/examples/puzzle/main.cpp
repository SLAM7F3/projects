#include <QtGui>
#include "puzzle.h"

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    Puzzle* puzzle = new Puzzle;
    QPushButton* next = new QPushButton( "Next" );
    QObject::connect( next, SIGNAL( clicked() ), puzzle, SLOT( showNext() ) );

    QHBoxLayout* hlay = new QHBoxLayout;
    hlay->addStretch(1);
    hlay->addWidget( next);

    QVBoxLayout* vlay = new QVBoxLayout;
    vlay->addWidget( puzzle );
    vlay->addLayout( hlay );

    QWidget* widget = new QWidget;
    widget->setLayout( vlay );
    widget->show();

    return app.exec();
}
