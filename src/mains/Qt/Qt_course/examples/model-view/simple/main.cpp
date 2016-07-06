#include <QtGui>

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    QSplitter* splitter = new QSplitter;
    QTreeView* tree = new QTreeView( splitter );
    QListView* list = new QListView( splitter );

    QDirModel* model = new QDirModel;
    tree->setModel( model );
    tree->setRootIndex( model->index( QDir::currentPath() ) );
    list->setModel( model );
    list->setRootIndex( model->index( QDir::currentPath() ) );

    QItemSelectionModel* selection = new QItemSelectionModel( model );
    tree->setSelectionModel( selection );
    list->setSelectionModel( selection );

    splitter->show();

    return app.exec();
}
