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



int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    // Connect to the database
    QSqlDatabase db = QSqlDatabase::addDatabase( "QMYSQL" );
    db.setDatabaseName(s_databaseName);
    db.setUserName(s_user);
    db.setPassword(s_password);
    if ( !db.open() )
        reportError( "Error When opening database", db.lastError() );

    QSqlQueryModel* model = new QSqlQueryModel;
    model->setQuery( "SELECT a.firstname, a.surname, b.title, b.price, b.notes FROM author a, book b WHERE a.id = b.authorid" );

    model->setHeaderData( 0, Qt::Horizontal, "First Name" );
    model->setHeaderData( 1, Qt::Horizontal, "Sur Name" );
    model->setHeaderData( 2, Qt::Horizontal, "Title" );
    model->setHeaderData( 3, Qt::Horizontal, "Price" );
    model->setHeaderData( 4, Qt::Horizontal, "Notes" );

    QTableView* view = new QTableView;
    view->setModel( model );
    view->show();

    return app.exec();
}
