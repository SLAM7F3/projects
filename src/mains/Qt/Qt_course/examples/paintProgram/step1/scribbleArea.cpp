#include <QtGui>
#include "scribbleArea.h"

ScribbleArea::ScribbleArea( QWidget* parent ) : QWidget( parent )
{
}


/**
  * This virtual method gets called whenever the users presses the
  * mouse over the window. It just records the position of the mouse
  * at the time of the click.
  */
void ScribbleArea::mousePressEvent( QMouseEvent* event )
{
  _last = event->pos(); // retrieve the coordinates from the event
}


/**
  * This virtual method gets called whenever the user moves the mouse
  * while the mouse button is pressed (this is also known as
  * "dragging"). If we had called setMouseTracking( true ) before,
  * this method would also be called when the mouse was moved without
  * any button pressed. We know that we haven't, and thus don't have
  * to check whether any buttons are pressed.
  */
void ScribbleArea::mouseMoveEvent( QMouseEvent* event )
{
  QPainter painter( &_buffer );
  painter.setPen( QPen( Qt::black, 3 ) );
  painter.drawLine( _last, event->pos() );
  update();

  // remember the current mouse position
  _last = event->pos();
}



/**
  * This virtual method gets called whenever the widget needs
  * painting, e.g., when it has been obscured and unhidden again,
  * and when repaint() is called.
  */
void ScribbleArea::paintEvent( QPaintEvent* )
{
    // copy the image from the buffer pixmap to the window
    QPainter painter( this );
    painter.drawPixmap( 0,0, _buffer );
}


/**
  * This virtual method gets called whenever the window is resized. We
  * use it to make sure that the off-screen buffer always has the same
  * size as the window.
  * In order not to lose the original scribbling, it is first copied
  * to a temporary buffer. After the main buffer has been resized and
  * filled with white, the image is copied from the temporary buffer to
  * the main buffer.
  */
void ScribbleArea::resizeEvent( QResizeEvent* event )
{
    QPixmap tmp( event->size() );
    tmp.fill( Qt::white );

    QPainter painter( &tmp );
    painter.drawPixmap( 0,0, _buffer );
    painter.end();
    _buffer = tmp;
}
