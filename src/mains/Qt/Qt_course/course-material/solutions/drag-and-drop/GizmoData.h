#ifndef GIZMODATA_H
#define GIZMODATA_H

#include <QMimeData>
class Gizmo;

class GizmoData :public QMimeData
{
public:
    GizmoData( Gizmo* gizmo );
    virtual QStringList formats () const;
    static bool decode( Gizmo* destination, const QMimeData* data );

protected:
    virtual QVariant retrieveData( const QString & mimetype, QVariant::Type type ) const;

private:
    QColor _col1, _col2;
    Qt::Orientation _orientation;
};

#endif /* GIZMODATA_H */

