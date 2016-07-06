#ifndef OBJECTBROWSERMODEL_H
#define OBJECTBROWSERMODEL_H

#include <QAbstractItemModel>

class ObjectBrowserModel :public QAbstractItemModel
{
public:
    ObjectBrowserModel();
    virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    virtual QModelIndex parent ( const QModelIndex & index ) const;
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

protected:
    QList<QObject*> children( const QModelIndex& parent ) const;
    QList<QObject*> children( QObject* parent ) const;
    QString dataForColumn( QObject* obj, int column ) const;
};

#endif /* OBJECTBROWSERMODEL_H */

