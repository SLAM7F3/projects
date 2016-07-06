#include <QtDebug>

#include "ModelFlipProxy.h"

ModelFlipProxy::ModelFlipProxy( QObject* parent )
    : QAbstractProxyModel( parent )
{
    // TODO: reimplement setSourceModel, connect to dataChanged and forward after adjusting the index
}

ModelFlipProxy::~ModelFlipProxy()
{
}

QModelIndex ModelFlipProxy::mapFromSource( const QModelIndex& sourceIndex ) const
{
    Q_ASSERT( !sourceIndex.isValid() || sourceIndex.model() == sourceModel() );
    return createIndex( sourceIndex.column(), sourceIndex.row() );
}

QModelIndex ModelFlipProxy::mapToSource( const QModelIndex& proxyIndex ) const
{
    Q_ASSERT( !proxyIndex.isValid() || proxyIndex.model() == this );
    return sourceModel()->index( proxyIndex.column(), proxyIndex.row() );
}

QModelIndex ModelFlipProxy::index(int row, int column, const QModelIndex& index) const
{
    Q_UNUSED( index );
    Q_ASSERT( !index.isValid() );
    return createIndex( row, column );
}

QModelIndex ModelFlipProxy::parent(const QModelIndex& index) const
{
    Q_UNUSED( index );
    return QModelIndex();
}

int ModelFlipProxy::rowCount(const QModelIndex& index) const
{
    Q_ASSERT( !index.isValid() );
    return sourceModel()->columnCount( index );
}

int ModelFlipProxy::columnCount(const QModelIndex& index) const
{
    Q_ASSERT( !index.isValid() );
    return sourceModel()->rowCount( index );
}

QVariant ModelFlipProxy::data(const QModelIndex& index, int role) const
{
    Q_ASSERT( !index.isValid() || index.model() == this );
    return sourceModel()->data( mapToSource( index ), role );
}

bool ModelFlipProxy::setData( const QModelIndex & index, const QVariant & value, int role )
{
    return sourceModel()->setData( mapToSource( index ), value, role );
}

QVariant ModelFlipProxy::headerData( int section, Qt::Orientation orientation, int role ) const
{
    return sourceModel()->headerData( section, orientation == Qt::Horizontal ? Qt::Vertical : Qt::Horizontal, role );
}
