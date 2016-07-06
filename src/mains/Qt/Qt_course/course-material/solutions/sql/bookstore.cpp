#include <QtGui>
#include <QtSql>
#include "bookstore.h"
#include "bookmodel.h"

BookStore::BookStore()
{
    // Connect to the database
    QSqlDatabase db = QSqlDatabase::addDatabase( "QMYSQL3" );
    db.setDatabaseName("bookstore");
    if ( !db.open() )
        reportError( "Error When opening database", db.lastError() );

    // Set up the models
    _authorsModel = new QSqlTableModel( this );
    _authorsModel->setTable( "author" );
    _authorsModel->select();
    _authorsModel->setHeaderData( 1, Qt::Horizontal, "First Name" );
    _authorsModel->setHeaderData( 2, Qt::Horizontal, "Sur Name" );

    _bookModel = new BookModel( this );

    // The GUI
    QSplitter* splitter = new QSplitter( Qt::Vertical, this );
    _authorView = new QTableView( splitter );
    _authorView->setModel( _authorsModel );
    _authorView->setColumnHidden( 0, true ); // Don't show ID column
    _authorView->verticalHeader()->hide();

    _bookView = new QTableView( splitter );
    _bookView->setModel( _bookModel );
    _bookView->verticalHeader()->hide();

    connect( _authorView->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
             this, SLOT( authorChanged( const QModelIndex& ) ) );

    _authorView->installEventFilter( this );
    _bookView->installEventFilter( this );

    // The Layout
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget( splitter );
    setLayout( layout );

    // Select the first item in the author model.
    _authorView->selectionModel()->setCurrentIndex( _authorsModel->index( 0, 1, QModelIndex() ), QItemSelectionModel::Select );
}

void BookStore::reportError( const QString& msg, const QSqlError& err )
{
    qDebug() <<
        QString("%1\nDriver Message: %2\nDatabase Message %3")
        .arg(msg)
        .arg(err.driverText())
        .arg(err.databaseText());
    qApp->exit(-1);
}

void BookStore::authorChanged( const QModelIndex& index )
{
    int authorId = _authorsModel->data( _authorsModel->index( index.row(), 0, QModelIndex() ) ).toInt();
    _bookModel->showAuthor( authorId );
    _bookView->setColumnHidden( 0, true ); // Don't show the ID column
}

bool BookStore::eventFilter( QObject* watched, QEvent* event )
{
    if ( event->type() == QEvent::ContextMenu ) {
        QModelIndex index;
        QAbstractItemModel* model;
        QString item;
        if ( watched == _authorView ) {
            index = _authorView->indexAt( _authorView->viewport()->mapFromGlobal( QCursor::pos() ) );
            QString firstName = _authorsModel->data( _authorsModel->index( index.row(), 1 ) ).toString();
            QString surName =  _authorsModel->data( _authorsModel->index( index.row(), 2 ) ).toString();
            item = QString( "%1 %2" ).arg( firstName ).arg( surName );
            model = _authorsModel;
        }
        else if ( watched == _bookView ) {
            index = _bookView->indexAt( _bookView->viewport()->mapFromGlobal( QCursor::pos() ) );
            item = _bookModel->data( _bookModel->index( index.row(), 1 ) ).toString();
            model = _bookModel;
        }
        else
            return false;

        QMenu* menu = new QMenu;
        QAction* add = menu->addAction( "Add New Item" );
        QAction* del = menu->addAction( QString( "Delete %1" ).arg( item ) );
        del->setEnabled( index.isValid() );

        QAction* action = menu->exec( QCursor::pos() );

        if ( action == add ) {
            if ( index.isValid() )
                model->insertRow( index.row(), index.parent() );
            else
                model->insertRow( model->rowCount(), QModelIndex() );
        }
        else if ( action == del ) {
            model->removeRow( index.row(), index.parent() );
        }

        return true;
    }
    return false;
}
