#include <QtGui>
#include "gizmo.h"

int main( int argc, char ** argv ) {
    QApplication app( argc, argv );

    Gizmo* gizmo1 = new Gizmo( Qt::blue, Qt::green, Qt::Horizontal );
    Gizmo* gizmo2 = new Gizmo( Qt::yellow, Qt::black, Qt::Vertical );
    Gizmo* gizmo3 = new Gizmo( Qt::white, Qt::green, Qt::Horizontal );

    QWidget *top = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout( top );

    layout->addWidget( gizmo1 );
    layout->addWidget( gizmo2 );
    layout->addWidget( gizmo3 );

    top->show();
    return app.exec();
}
