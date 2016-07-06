#include <QtGui>
#include "gizmo.h"
#include "GizmoData.h"

Gizmo::Gizmo( const QColor& col1, const QColor& col2, Qt::Orientation orientation, QWidget* parent )
    : QFrame( parent )
{
    _col1 = col1;
    _col2 = col2;
    _orientation = orientation;
    setAcceptDrops( true );
    setFocusPolicy( Qt::StrongFocus );
}

void Gizmo::paintEvent( QPaintEvent * )
{
    paint( this );
}


void Gizmo::paint( QPaintDevice* device )
{
    int width = device->width();
    int height = device->height();
    QRect rect1;
    QRect rect2;

    if ( _orientation == Qt::Vertical ) {
        rect1 = QRect( 0, 0, width/2, height);
        rect2 = QRect( width/2, 0, width/2, height);
    }
    else {
        rect1 = QRect( 0, 0, width, height/2);
        rect2 = QRect( 0, height/2, width, height/2);
    }

    QPainter painter(device);

    painter.setPen(_col1);
    painter.setBrush( QBrush( _col1, Qt::SolidPattern) );
    painter.drawRect( rect1 );

    painter.setPen(_col2);
    painter.setBrush( QBrush( _col2, Qt::SolidPattern) );
    painter.drawRect( rect2 );

    if ( hasFocus() ) {
        QRect rect( 0,0, width-1, height-1);
        painter.setBrush( Qt::NoBrush );
        QPen pen( Qt::white );
        painter.setPen( pen );
        painter.drawRect( rect );

        pen.setColor( Qt::black );
        pen.setStyle( Qt::DotLine );
        painter.setPen( pen );
        painter.drawRect( rect );
    }
}

QSize Gizmo::sizeHint() const
{
    return QSize( 100, 100 );
}

void Gizmo::mousePressEvent( QMouseEvent* e )
{
    _dragStartPos = e->pos();
}

void Gizmo::mouseMoveEvent( QMouseEvent* e )
{
    if ( e->buttons() & Qt::LeftButton &&
         (e->pos() - _dragStartPos).manhattanLength() > QApplication::startDragDistance() ) {
        QDrag* drag = new QDrag( this );
        GizmoData *mimeData = new GizmoData( this );
        drag->setMimeData( mimeData );

        QPixmap pixmap( width(), height() );
        paint( &pixmap );
        drag->setPixmap( pixmap );

        drag->start();
    }
}

void Gizmo::dragEnterEvent( QDragEnterEvent * e )
{
    if ( e->mimeData()->hasFormat( "x-gizmo/x-drag" ) )
        e->accept();
}

void Gizmo::dropEvent( QDropEvent * e )
{
    if ( GizmoData::decode( this, e->mimeData() ) ) {
        e->acceptProposedAction();
        repaint();
    }
}

void Gizmo::keyPressEvent( QKeyEvent* event )
{
    if ( event->key() == Qt::Key_C && event->modifiers() & Qt::ControlModifier ) {
        QApplication::clipboard()->setMimeData( new GizmoData( this ) );
    }

    else if ( event->key() == Qt::Key_V && event->modifiers() & Qt::ControlModifier ) {
        GizmoData::decode( this, QApplication::clipboard()->mimeData() );
        repaint();
    }
}
