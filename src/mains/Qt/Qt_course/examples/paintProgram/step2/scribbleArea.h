#ifndef SCRIBBLEAREA_H
#define SCRIBBLEAREA_H

#include <QWidget>
#include <QPixmap>
#include <QPoint>
#include <QColor>
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
    explicit ScribbleArea( QWidget* parent=0 );

protected:
    virtual void mousePressEvent( QMouseEvent* );
    virtual void mouseMoveEvent( QMouseEvent* );
    virtual void paintEvent( QPaintEvent* );
    virtual void resizeEvent( QResizeEvent* );

public slots:
    void slotChangeColor( const QColor & );
    void setPenStyle( Qt::PenStyle style );

private:
    QPoint _last;
    QColor _color;
    QPixmap _buffer;
    Qt::PenStyle _penStyle;
};

#endif /* SCRIBBLEAREA_H */

