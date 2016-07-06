#include <QtGui>
#include "GizmoData.h"
#include "gizmo.h"

GizmoData::GizmoData( Gizmo* gizmo )
{
    _col1 = gizmo->_col1;
    _col2 = gizmo->_col2;
    _orientation = gizmo->_orientation;
}

QVariant GizmoData::retrieveData( const QString & mimetype, QVariant::Type type ) const
{
    if ( mimetype == "text/plain" ) {
        QString str = QString( "(%1, %2, %3)" )
                      .arg( _col1.name() )
                      .arg( _col2.name() )
                      .arg( _orientation == Qt::Horizontal ? "Horizontal" : "Vertical" );

        if ( type == QVariant::String )
            return str;

        else if ( type == QVariant::ByteArray )
            return str.toAscii();

        else {
            qDebug() << "Don't know how to handle text/plain with type " << type;
            return QVariant();
        }
    }
    else if ( mimetype == "x-gizmo/x-drag" ) {
        Q_ASSERT( type == QVariant::ByteArray );
        QByteArray data;
        QDataStream stream( &data, QIODevice::WriteOnly );
        stream << _col1 << _col2 << (int) _orientation;
        return data;
    }

    return QVariant();
}

bool GizmoData::decode( Gizmo* destination, const QMimeData* mime )
{
    if ( mime->hasFormat( "x-gizmo/x-drag" ) ) {
        QByteArray data = mime->data( "x-gizmo/x-drag" );
        QDataStream stream( data );
        stream >> destination->_col1 >> destination->_col2 >> (int&) destination->_orientation;
        return true;
    }
    return false;
}

QStringList GizmoData::formats() const
{
    return QStringList() << "text/plain" << "x-gizmo/x-drag";
}
