#include <QtGui>

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    QStandardItemModel* model = new QStandardItemModel( 10, 10 );
    for ( int toprow = 0; toprow < 5; ++toprow ) {
        for ( int topcol = 0; topcol < 10; ++topcol ) {
            model->setData( model->index( toprow, topcol ), QString( "(%1,%2)").arg(toprow).arg(topcol) );
        }
        QModelIndex index = model->index( toprow, 0 );

        model->insertRows(0, 10, index);
        model->insertColumns(0, 10, index);

        for ( int row = 0; row < 10; ++row )
            for ( int col = 0; col < 10; ++col )
                model->setData( model->index( row, col, index ), row*col );
    }


    QTableView* table = new QTableView;
    table->setModel( model );
    table->show();

    QTreeView* tree = new QTreeView;
    tree->setModel( model );
    tree->show();

    return app.exec();
}
