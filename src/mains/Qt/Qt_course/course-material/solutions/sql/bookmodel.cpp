#include <QtGui>
#include <QtSql>
#include "bookmodel.h"
#include "bookstore.h"
BookModel::BookModel( QObject* parent )
    : QSqlQueryModel( parent )
{
    setHeaderData( 1, Qt::Horizontal, "Title" );
    setHeaderData( 2, Qt::Horizontal, "Price" );
    setHeaderData( 3, Qt::Horizontal, "Notes" );
}

void BookModel::showAuthor( int authorId )
{
    _authorId = authorId;
    setQuery( QString( "SELECT id, title, price, notes FROM book WHERE authorid = %1" ).arg( authorId ) );
    if ( lastError().type() != QSqlError::NoError )
        BookStore::reportError( "Error running showAuthor query", lastError() );
}

bool BookModel::setData( const QModelIndex& idx, const QVariant& value, int /*role*/ )
{
    QModelIndex primaryKeyIndex = index( idx.row(), 0 , idx.parent() );
    int primaryKey = data( primaryKeyIndex ).toInt();

    QString field;
    switch ( idx.column() ) {
    case 1: field = "title"; break;
    case 2: field = "price"; break;
    case 3: field = "notes"; break;
    default: qFatal( "Unknown field number %d", idx.column() );
    }

    QSqlQuery query;
    query.prepare( QString( "update book set %1 = :value where id = :id" ).arg( field ) );
    query.bindValue( ":value", value );
    query.bindValue( ":id", primaryKey );
    bool ok = query.exec();
    if ( !ok )
        BookStore::reportError( "Error running update command", query.lastError() );
    refresh();
    return ok;
}

Qt::ItemFlags BookModel::flags( const QModelIndex& index ) const
{
    return QSqlQueryModel::flags( index ) | Qt::ItemIsEditable;
}

bool BookModel::removeRows( int row, int count, const QModelIndex& parent  )
{
    for ( int i=0; i< count; ++i ) {
        QModelIndex primaryKeyIndex = index( row, 0 , parent );
        int primaryKey = data( primaryKeyIndex ).toInt();

        QSqlQuery query;
        query.prepare( QString( "DELETE FROM book where id = :id" ) );
        query.bindValue( ":id", primaryKey );
        bool ok = query.exec();
        if ( !ok ) {
            BookStore::reportError( "Error running update command", query.lastError() );
            return false;
        }
        refresh();
    }
    return true;
}

bool BookModel::insertRows( int /*row*/, int count, const QModelIndex &/*parent*/ )
{
    for ( int i=0; i < count; ++i ) {
        QSqlQuery query;
        query.prepare( QString( "INSERT INTO book set authorId = :id" ) );
        query.bindValue( ":id", _authorId );
        bool ok = query.exec();
        if ( !ok ) {
            BookStore::reportError( "Error running update command", query.lastError() );
            return false;
        }
        refresh();
    }
    return true;

}

void  BookModel::refresh()
{
    showAuthor( _authorId );
}
