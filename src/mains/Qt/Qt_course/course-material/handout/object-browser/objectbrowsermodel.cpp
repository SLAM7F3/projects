#include <QtGui>
#include "objectbrowsermodel.h"

QModelIndex ObjectBrowserModel::parent( const QModelIndex & index ) const
{
    return QModelIndex(); // Just for now.

    // This is the implementation needed when you implement the real version of index()
    QObject* obj = static_cast<QObject*>( index.internalPointer() );
    if ( obj->isWidgetType() && static_cast<QWidget*>(obj)->parent() == 0 )
        return QModelIndex(); // Top most index.
    else {
        QList<QObject*> siblings = children( obj->parent()->parent() );
        return createIndex( siblings.indexOf( obj->parent() ), 0, obj->parent() );
    }
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
