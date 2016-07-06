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


int main( int argc, char** argv )
{
  QApplication app(argc, argv);

  // Connect to the database
  QSqlDatabase db = QSqlDatabase::addDatabase( "QMYSQL" );
    db.setDatabaseName(s_databaseName);
    db.setUserName(s_user);
    db.setPassword(s_password);
  if ( !db.open() )
    reportError( "Error When opening database", db.lastError() );

  // Setup the listview.
  QTreeWidget* lv = new QTreeWidget;
  lv->setHeaderLabels( QStringList() << "Name" << "Price" << "Notes");
  lv->setRootIsDecorated( true );

  // Query the database
  QSqlQuery authorQuery("SELECT id, firstname, surname FROM author");
  if ( ! authorQuery.isActive() )
    reportError("Eror when running Query", authorQuery.lastError());

  while ( authorQuery.next() ) {
    QString name = authorQuery.value(1).toString() + " " + authorQuery.value(2).toString();
    QTreeWidgetItem* authorItem = new QTreeWidgetItem( lv );
    authorItem->setText( 0, name );

    // Query all the books of the author.
    QSqlQuery bookQuery( "SELECT title, price, notes FROM book WHERE authorid = " + authorQuery.value(0).toString());

    if ( ! bookQuery.isActive() )
      reportError("Eror when running Query",bookQuery.lastError());

    while ( bookQuery.next() ) {
        QTreeWidgetItem* book = new QTreeWidgetItem( authorItem );
        book->setText( 0, bookQuery.value(0).toString() );
        book->setText( 1, bookQuery.value(1).toString() );
        book->setText( 2, bookQuery.value(2).toString() );
    }
  }

  lv->show();

  return app.exec();
}
