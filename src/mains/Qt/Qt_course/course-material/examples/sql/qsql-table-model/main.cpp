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

    QSqlTableModel* model = new QSqlTableModel;
    model->setTable("book");
    model->select();


    QTableView* view = new QTableView;
    view->setModel( model );
    view->show();

    model->setEditStrategy(QSqlTableModel::OnManualSubmit); // Avoid round trip for each change.
    // add 10% to all prices
    for ( int row = 0; row < model->rowCount(); ++row ) {
        QSqlRecord record = model->record( row );
        double price = record.value( "price" ).toDouble();
        price *= 1.1;
        record.setValue( "price", price );
        model->setRecord( row, record );
    }
    model->submitAll();
    model->setEditStrategy(QSqlTableModel::OnRowChange);

    return app.exec();
}
