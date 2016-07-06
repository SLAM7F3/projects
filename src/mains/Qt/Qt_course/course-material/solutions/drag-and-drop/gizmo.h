#ifndef GIZMO_H
#define GIZMO_H

#include <QFrame>
#include <QColor>
#include <QPoint>
class QPaintEvent;

class Gizmo :public QFrame
{
public:
    Gizmo( const QColor& , const QColor&, Qt::Orientation, QWidget* parent = 0 );
    QSize sizeHint() const;
    void paint( QPaintDevice* device );

protected:
    virtual void paintEvent( QPaintEvent * );
    virtual void mousePressEvent( QMouseEvent* );
    virtual void mouseMoveEvent( QMouseEvent* );
    virtual void dragEnterEvent( QDragEnterEvent * event );
    virtual void dropEvent ( QDropEvent * event );
    virtual void keyPressEvent( QKeyEvent* event );

public:
    QColor _col1, _col2;
    Qt::Orientation _orientation;
private:
    QPoint _dragStartPos;
};



#endif /* GIZMO_H */

