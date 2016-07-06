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

    QSqlRelationalTableModel* model = new QSqlRelationalTableModel;
    model->setTable("book");
    model->setRelation( 3, QSqlRelation( "author", "id", "surname" ) );
    model->select();

    QTableView* view = new QTableView;
    view->setModel( model );
    view->show();

    return app.exec();
}
