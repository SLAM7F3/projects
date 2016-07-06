#include <QApplication>
#include <QStandardItemModel>

#include "SortedTableView.h"

struct ModelData {
    QString name;
    QString nickname;
} Dudes[] = {
    { "M. K. Dalheimer", "BigBoss" },
    { "Jesper K. Pedersen",  "Blackie" },
    { "Mirko Boehm", "Miroslav" },
    { "David Faure",  "dfaure" }
};

const int NoOfDudes = sizeof( Dudes )/sizeof ( ModelData );

int main ( int argc,  char** argv )
{

    QApplication app ( argc, argv );
    SortedTableView table;
    QStandardItemModel model;
    model.insertRows ( 0, NoOfDudes, QModelIndex() );
    model.insertColumns ( 0, 2, QModelIndex() );
    for ( int row = 0; row < NoOfDudes; ++ row )
    {
        model.setData ( model.index( row, 0, QModelIndex() ), Dudes[row].name );
        model.setData ( model.index( row, 1, QModelIndex() ), Dudes[row].nickname );
    }

    table.setModel ( &model );
    table.show();

    return app.exec();
}
