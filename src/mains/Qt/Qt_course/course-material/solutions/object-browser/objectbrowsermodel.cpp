#include <QtGui>
#include "objectbrowsermodel.h"
ObjectBrowserModel::ObjectBrowserModel()
{

}

int ObjectBrowserModel::columnCount( const QModelIndex & /*parent*/ ) const
{
    return 3;
}

QVariant ObjectBrowserModel::data( const QModelIndex & index, int role ) const
{
    if ( role == Qt::DisplayRole ) {
        QObject* obj = static_cast<QObject*>( index.internalPointer() );
        int column = index.column();
        return dataForColumn( obj, column );
    }

    if ( role == Qt::UserRole )
        return QVariant( QVariant::UserType, index.internalPointer() );

    return QVariant();
}

QModelIndex ObjectBrowserModel::index( int row, int column, const QModelIndex & parent ) const
{
    QList<QObject*> list = children( parent );
    if ( list.count() > row )
        return createIndex( row, column, list[row] );
    else
        return QModelIndex();
}

QModelIndex ObjectBrowserModel::parent( const QModelIndex & index ) const
{
    QObject* obj = static_cast<QObject*>( index.internalPointer() );
    if ( obj->isWidgetType() && static_cast<QWidget*>(obj)->parent() == 0 )
        return QModelIndex(); // Top most index.
    else {
        QList<QObject*> siblings = children( obj->parent()->parent() );
        return createIndex( siblings.indexOf( obj->parent() ), 0, obj->parent() );
    }
}

int ObjectBrowserModel::rowCount( const QModelIndex & parent ) const
{
    return children( parent ).count();
}

QList<QObject*> ObjectBrowserModel::children( const QModelIndex& parent ) const
{
    if ( !parent.isValid() )
        return children( (QObject*) 0 );
    else
        return children( static_cast<QObject*>( parent.internalPointer() ) );
}

QList<QObject*> ObjectBrowserModel::children( QObject* parent ) const
{
    QList<QObject*> result;
    if ( parent == 0 ) {
        QList<QWidget*> widgets = QApplication::topLevelWidgets();

        foreach ( QWidget* widget, widgets )
            if ( widget->isVisible() && widget->windowType() != Qt::Desktop)
                result.append( widget );
    }
    else
        result = parent->children();

    qSort( result );
    return result;
}

QVariant ObjectBrowserModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if ( role != Qt::DisplayRole || orientation != Qt::Horizontal )
        return QVariant();

    switch (section) {
    case 0: return "Class Name";
    case 1: return "Object Name";
    case 2: return "Pointer";
    }

    return QVariant();
}

QString ObjectBrowserModel::dataForColumn( QObject* obj, int column ) const
{
    switch ( column ) {
    case 0:
        return obj->metaObject()->className();
    case 1:
        return obj->objectName();
    case 2:
        QString str;
        str.sprintf("%p", obj);
        return str ;
    }
    return QString();
}
