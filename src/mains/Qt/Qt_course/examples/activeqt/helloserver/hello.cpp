#include "hello.h"
#include <QtGui>

/*
  Constructs a Hello widget. Starts a 40 ms animation timer.
*/
Hello::Hello( QWidget *parent )
    : QWidget(parent), _text("Hello"), b(0)
{
    _timer = new QTimer(this);
    connect( _timer, SIGNAL(timeout()), SLOT(animate()) );

    //resize( 260, 130 );
	start();
}

void Hello::start()
{
    _timer->start( 40 );
}

void Hello::stop()
{
	_timer->stop();
}

/*
  This private slot is called each time the timer fires.
*/

void Hello::animate()
{
    b = (b + 1) & 15;
    repaint();
}

void Hello::setText( const QString& txt )
{
	_text = txt;
	update();
}

QString Hello::text() const 
{ 
	return _text; 
}

/*
  Handles mouse button release events for the Hello widget.

  We emit the clicked() signal when the mouse is released inside
  the widget.
*/

void Hello::mouseReleaseEvent( QMouseEvent *e )
{
    if ( rect().contains( e->pos() ) )
        emit clicked();
}


/*
  Handles paint events for the Hello widget.

  Flicker-free update. The text is first drawn in the pixmap and the
  pixmap is then blt'ed to the screen.
*/

void Hello::paintEvent( QPaintEvent * )
{
    static int sin_tbl[16] = {
        0, 38, 71, 92, 100, 92, 71, 38,	0, -38, -71, -92, -100, -92, -71, -38};

    if ( text().isEmpty() )
        return;

    // 1: Compute some sizes, positions etc.
    QFontMetrics fm = fontMetrics();
    int w = fm.width(text()) + 20;
    int h = fm.height() * 2;

    // 2: Paint the pixmap. Cool wave effect
    QPainter p(this);
    int x = 10;
    int y = h/2 + fm.descent();
    int i = 0;
    p.setFont( font() );
    while ( !text()[i].isNull() ) {
        int i16 = (b+i) & 15;
		QColor color;
		color.setHsv((15-i16)*16,255,255);
		p.setPen( color );
        p.drawText( x+width()/2-w/2, y-sin_tbl[i16]*h/800+height()/2-h/2, text().mid(i,1) );
        x += fm.width( text()[i] );
        i++;
    }
    p.end();
}
