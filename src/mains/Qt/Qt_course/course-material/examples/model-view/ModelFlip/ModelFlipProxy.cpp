#include <QtDebug>

#include "ModelFlipProxy.h"

ModelFlipProxy::ModelFlipProxy ( QObject* parent )
    : QAbstractProxyModel ( parent )
{
}

ModelFlipProxy::~ModelFlipProxy()
{
}

QModelIndex ModelFlipProxy::mapFromSource ( const QModelIndex& sourceIndex ) const
{
    return createIndex ( sourceIndex.column(), sourceIndex.row(),
                         sourceIndex.internalPointer() );
}

QModelIndex ModelFlipProxy::mapToSource ( const QModelIndex& proxyIndex ) const
{
    return createIndex ( proxyIndex.column(), proxyIndex.row(),
                         proxyIndex.internalPointer() );
}


QModelIndex ModelFlipProxy::index(int row, int column, const QModelIndex& index) const
{
    Q_UNUSED ( index );
    return createIndex( row,  column );
}

QModelIndex ModelFlipProxy::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

int ModelFlipProxy::rowCount(const QModelIndex& index) const
{
    return sourceModel()->columnCount( index );
}

int ModelFlipProxy::columnCount(const QModelIndex& index) const
{
    return sourceModel()->rowCount( index );
}

QVariant ModelFlipProxy::data(const QModelIndex& index, int role) const
{
    return sourceModel()->data( mapToSource( index ), role );
}

