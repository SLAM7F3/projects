#ifndef MODELFLIPPROXY_H
#define MODELFLIPPROXY_H

#include <QAbstractProxyModel>

class ModelFlipProxy : public QAbstractProxyModel
{
    Q_OBJECT
public:
    explicit ModelFlipProxy ( QObject* parent = 0 );
    ~ModelFlipProxy ();

    QModelIndex mapFromSource ( const QModelIndex & sourceIndex ) const;
    QModelIndex mapToSource ( const QModelIndex & proxyIndex ) const;

    QModelIndex index(int, int, const QModelIndex&) const;
    QModelIndex parent(const QModelIndex&) const;
    int rowCount(const QModelIndex&) const;
    int columnCount(const QModelIndex&) const;
    QVariant data(const QModelIndex&, int) const;
};

#endif
