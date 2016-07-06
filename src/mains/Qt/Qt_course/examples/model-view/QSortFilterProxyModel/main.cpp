#include <QApplication>
#include <QStandardItemModel>

#include "SortedTableView.h"

static const struct ModelData {
    QString name;
    QString nickname;
    int age; // fake numbers :)
} Dudes[] = {
    { "M. K. Dalheimer", "Pointy Haired Boss", 33 },
    { "Jesper K. Pedersen",  "Blackie", 23 },
    { "Mirko Boehm", "Miroslav", 26 },
    { "David Faure",  "dfaure", 9 } // small number, to test that sorting isn't done using QString
};

static const int NoOfDudes = sizeof( Dudes )/sizeof ( ModelData );

int main( int argc,  char** argv )
{
    QApplication app( argc, argv );
    SortedTableView table;
    QStandardItemModel model;
    model.insertRows( 0, NoOfDudes, QModelIndex() );
    model.insertColumns( 0, 3, QModelIndex() );
    for ( int row = 0; row < NoOfDudes; ++ row )
    {
        model.setData( model.index( row, 0, QModelIndex() ), Dudes[row].name );
        model.setData( model.index( row, 1, QModelIndex() ), Dudes[row].nickname );
        model.setData( model.index( row, 2, QModelIndex() ), Dudes[row].age );
    }
    model.setHeaderData( 0, Qt::Horizontal, "Name" );
    model.setHeaderData( 1, Qt::Horizontal, "Nickname" );
    model.setHeaderData( 2, Qt::Horizontal, "Age" );

    table.setModel( &model );
    table.resize( 500, 400 ); // no sizeHint in QTableView in Qt-4.2
    table.show();

    return app.exec();
}
