#include <QPen>
#ifndef SCRIBBLEAREA_H
#define SCRIBBLEAREA_H

#include <QWidget>
#include <QPixmap>
#include <QPoint>
class QResizeEvent;
class QPaintEvent;
class QMouseEvent;

/**
 * A class that lets the user draw scribbles with the mouse. The
 * window knows how to redraw itself.
 */
class ScribbleArea : public QWidget
{
    Q_OBJECT

public:
    ScribbleArea( QWidget* parent );
    enum Shape { Line, Rectangle, Ellipse, Image };

protected:
    virtual void mousePressEvent( QMouseEvent* );
    virtual void mouseMoveEvent( QMouseEvent* );
    virtual void paintEvent( QPaintEvent* );
    virtual void resizeEvent( QResizeEvent* );
    void drawArrow( QPainter& painter, const QPoint& start, const QPoint& end );

public slots:
    void slotChangeColor( const QColor & );
    void slotLoad( const QString& fileName );
    void slotSave( const QString& fileName );
    void slotPrint();
    void slotSetPen( int width, Qt::PenStyle style );
    void slotSetBrush( Qt::BrushStyle brush );
    void slotSetBrush( QPixmap brush );
    void setShape( int shape );
    void setPixmap( const QPixmap& pixmap );

private:
    QPoint _last;
    Shape _shape;
    QColor _color;
    QPixmap _buffer;
    QPen _pen;
    Qt::BrushStyle _brushStyle;
    QPixmap _pixmap;
};

#endif /* SCRIBBLEAREA_H */

