#include <QtGui>
#include <QtSql>
#include "../mysql_connect.h"

void reportError( const QString& msg, const QSqlError& err )
{
    qDebug() <<
        QString("%1\nDriver Message: %2\nDatabase Message %3")
        .arg(msg)
        .arg(err.driverText())
        .arg(err.databaseText());
    qApp->exit(-1);
}


class EditableQueryModel :public QSqlQueryModel
{
public:
    EditableQueryModel()
    {
        refresh();
        setHeaderData( 0, Qt::Horizontal, "First Name" );
        setHeaderData( 1, Qt::Horizontal, "Sur Name" );
        setHeaderData( 2, Qt::Horizontal, "Title" );
        setHeaderData( 3, Qt::Horizontal, "Price" );
        setHeaderData( 4, Qt::Horizontal, "Notes" );
    }

    Qt::ItemFlags flags( const QModelIndex& index ) const
    {
        Qt::ItemFlags flags = QSqlQueryModel::flags(index);
        if (index.column() >= 2 )
            flags |= Qt::ItemIsEditable;
        return flags;
    }

    bool setData(const QModelIndex &index, const QVariant& value, int /* role */)
    {
        Q_ASSERT(index.column()>= 2);

        QModelIndex primaryKeyIndex = QSqlQueryModel::index(index.row(), 5 );
        int id = data(primaryKeyIndex).toInt();

        QString field;
        switch ( index.column() ) {
        case 2: field = "title"; break;
        case 3: field = "price"; break;
        case 4: field = "notes"; break;
        }

        QSqlQuery query;
        query.prepare( QString("update book set %1 = ? where id  = ?" ).arg( field ) );
        query.addBindValue( value );
        query.addBindValue( id );
        bool ok = query.exec();
        if ( !ok )
            reportError("Error running update query", query.lastError() );
        refresh();
        return ok;
    }

    void refresh()
    {
        setQuery( "SELECT a.firstname, a.surname, b.title, b.price, b.notes, b.id  FROM author a, book b WHERE a.id = b.authorid" );
    }
};

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    // Connect to the database
    QSqlDatabase db = QSqlDatabase::addDatabase( "QMYSQL" );
    db.setDatabaseName(s_databaseName);
    db.setUserName(s_user);
    db.setPassword(s_password);
    if ( !db.open() )
        reportError( "Error When opening database", db.lastError() );

    EditableQueryModel* model = new EditableQueryModel;
    QTableView* view = new QTableView;
    view->setModel( model );
    view->setColumnHidden( 5, true ); // We do not want to see the id column
    view->show();

    return app.exec();
}
