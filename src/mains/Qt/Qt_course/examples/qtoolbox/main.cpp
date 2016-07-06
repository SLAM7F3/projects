#include <QtGui>

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    QToolBox* box = new QToolBox;
    box->addItem( new QDial(box), QPixmap("dial.png"), "dial");
    QTableWidget* table = new QTableWidget( 3,3, box );
    box->addItem( table, "table" );
    box->show();

    return app.exec();
}
