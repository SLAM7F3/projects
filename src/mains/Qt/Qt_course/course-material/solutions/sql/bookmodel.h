#ifndef BOOKMODEL_H
#define BOOKMODEL_H

#include <QSqlQueryModel>

class BookModel :public QSqlQueryModel
{
public:
    BookModel( QObject* parent );
    void showAuthor( int authorId );
    virtual bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );
    virtual Qt::ItemFlags flags( const QModelIndex& index ) const;
    virtual bool removeRows( int row, int count, const QModelIndex& parent = QModelIndex() );
    virtual bool insertRows( int row, int count, const QModelIndex & parent = QModelIndex() );

protected:
    void refresh();
private:
    int _authorId;
};

#endif /* BOOKMODEL_H */

