#ifndef OBJECTBROWSERMODEL_H
#define OBJECTBROWSERMODEL_H

#include <QAbstractItemModel>

class ObjectBrowserModel :public QAbstractItemModel
{
public:
    virtual QModelIndex parent ( const QModelIndex & index ) const;

protected:
    QList<QObject*> children( QObject* parent ) const;
};

#endif /* OBJECTBROWSERMODEL_H */

