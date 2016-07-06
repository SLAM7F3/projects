#include <QtGui>

class Test :public QMainWindow
{
    Q_OBJECT
public:
    Test( QWidget* parent = 0 ) :QMainWindow( parent )
    {
        _currentItem = new QLabel;
        statusBar()->addWidget( _currentItem );

        QTableView* view = new QTableView;
        setCentralWidget( view );

        _model = new QStandardItemModel( 10, 5, this );
        view->setModel( _model );

        QItemSelectionModel* selectionModel = view->selectionModel();
        connect( selectionModel, SIGNAL( currentChanged( const QModelIndex & , const QModelIndex &  ) ),
                 this, SLOT( currentItemChanged( QModelIndex ) ) );
        connect( selectionModel, SIGNAL( selectionChanged( const QItemSelection& , const QItemSelection&  ) ),
                 this, SLOT( updateSelection( const QItemSelection&, const QItemSelection&) ) );

        QItemSelection selection1( _model->index( 0, 0, QModelIndex() ), _model->index( 2, 2, QModelIndex() ) );
        selectionModel->select(  selection1, QItemSelectionModel::Select );


        QItemSelection selection2( _model->index( 6, 0, QModelIndex() ), _model->index( 8, 0, QModelIndex() ) );
        selectionModel->select(  selection2, QItemSelectionModel::Select | QItemSelectionModel::Rows );

        QItemSelection selection3( _model->index( 0, 1, QModelIndex() ), _model->index( 0, 1, QModelIndex() ) );
        selectionModel->select(  selection3,
                                 /*  QItemSelectionModel::Select | */
                                 QItemSelectionModel::Columns  | QItemSelectionModel::Toggle );
}

protected slots:
    void currentItemChanged(  const QModelIndex& current )
    {
        _currentItem->setText( QString( "row: %1 col: %2" ).arg( current.row() ).arg( current.column() ) );
    }
    void updateSelection( const QItemSelection& selected, const QItemSelection& deselected )
    {
        QModelIndexList items = selected.indexes();
        foreach( QModelIndex index, items )
            _model->setData( index, QString( "(%1,%2)" ).arg( index.row() ).arg( index.column() ) );

        items = deselected.indexes();
        foreach( QModelIndex index, items )
            _model->setData( index, "" );
    }


private:
    QLabel* _currentItem;
    QStandardItemModel* _model;
};

int main( int argc, char** argv ) {
    QApplication app( argc, argv );
    Test* test = new Test;
    test->resize( 800, 600 );
    test->show();

    return app.exec();
}

#include "main.moc"
