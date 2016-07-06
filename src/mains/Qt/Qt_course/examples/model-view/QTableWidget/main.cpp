#include <QtGui>

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    QTableWidget* w = new QTableWidget( 100, 5 );
    for ( int row = 0; row < 100; ++row ) {
        for ( int column = 0; column < 5; ++column )
            w->setItem( row, column, new QTableWidgetItem( QString( "%1" ).arg( row*column ) ) );
    }


    w->show();
    return app.exec();
}
