#include <QtGui>
#include "scribbleArea.h"
#include <cmath>

ScribbleArea::ScribbleArea( QWidget* parent )
    : QWidget( parent ), _shape( Line ), _pen( Qt::black, 3 ),_brushStyle( Qt::NoBrush )
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
    _start = event->pos();
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
    painter.setRenderHint( QPainter::Antialiasing );
    painter.drawPixmap( 0,0, _committed );

    painter.setPen( _pen );

    if ( _brushStyle == Qt::LinearGradientPattern ) {
        QLinearGradient gradient( _start, event->pos() );
        gradient.setColorAt( 0, Qt::red );
        gradient.setColorAt( 1, Qt::blue );
        painter.setBrush( gradient );
    }

    else if ( _brushStyle == Qt::ConicalGradientPattern ) {
        QConicalGradient gradient( (_start + event->pos()) / 2, 0 );
        gradient.setColorAt( 0, Qt::red );
        gradient.setColorAt( 1, Qt::blue );
        painter.setBrush( gradient );

    }

    else if ( _brushStyle == Qt::RadialGradientPattern ) {
        QRadialGradient gradient( (_start + event->pos()) / 2, (_start.x() + event->pos().x()) / 2, (_start + event->pos()) / 2 );
        gradient.setColorAt( 0, Qt::red );
        gradient.setColorAt( 1, Qt::blue );
        painter.setBrush( gradient );
    }

    else if ( _brushStyle == Qt::TexturePattern )
        painter.setBrush( QBrush( _color, _pixmap ) );

    else
        painter.setBrush( QBrush( _color, _brushStyle ) );



    if ( _shape == Line ) {
        drawArrow( painter, _start, event->pos() );
    }
    else if ( _shape == Rectangle ) {
        painter.drawRect( _start.x(), _start.y(), event->pos().x() - _start.x(),
                                event->pos().y() - _start.y());
    }
    else if ( _shape == Ellipse ) {
        painter.drawEllipse( _start.x(), _start.y(), event->pos().x() - _start.x(),
                                   event->pos().y() - _start.y());
    }
    else if ( _shape == Image ) {
        painter.drawPixmap( QRect(_start.x(), _start.y(), event->pos().x() -_start.x(),
                                        event->pos().y() - _start.y()).normalized(), _pixmap );
    }
    painter.end();

    repaint();
    _last = event->pos();
}

void ScribbleArea::mouseReleaseEvent( QMouseEvent* )
{
    _committed = _buffer;
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
    QPixmap tmp = QPixmap( event->size() );
    tmp.fill( Qt::white );
    QPainter painter( &tmp );
    painter.drawPixmap( 0,0, _buffer );
    painter.end();
    _buffer = tmp;

    tmp.fill( Qt::white );
    painter.begin( &tmp );
    painter.drawPixmap( 0,0, _committed );
    painter.end();
    _committed = tmp;
}

void ScribbleArea::slotChangeColor( const QColor& color )
{
    _color = color;
    _pen.setColor( color );
}

/**
 * This method does the actual loading. It relies on QPixmap (and the
 * underlying I/O machinery) to determine the filetype.
 */
void ScribbleArea::slotLoad( const QString& filename )
{
    if( !_buffer.load( filename ) )
        QMessageBox::warning( 0, "Load error", "Could not load file" );

    repaint();
}


/**
 * This method does the actual saving. We hardcode the file type as
 * BMP. Unix users might want to replace this with something more usual
 * on Unix like PNG.
 */
void ScribbleArea::slotSave( const QString& filename )
{
    if( !_buffer.save( filename, "BMP" ) )
        QMessageBox::warning( 0, "Save error", "Could not save file" );
}


/**
 * This method does the actual printing. It first opens a print dialog
 * and then prints the pixmap page by page. The implementation is
 * trivial, it just assumes that the image to print does not cover
 * more than one page. Page handling needs more code, but is not
 * fundamentally different.
 */
void ScribbleArea::slotPrint()
{
    QPrinter printer;
    QPrintDialog dialog( &printer, this );
    if( !dialog.exec() )
        return;
    QPainter ppainter( &printer );
    ppainter.drawPixmap( 0, 0,_buffer );
    printer.newPage();
}

void ScribbleArea::slotSetPen( int width, Qt::PenStyle style )
{
    _pen = QPen( _color, width, style );
}

void ScribbleArea::slotSetBrush( Qt::BrushStyle brushStyle )
{
    _brushStyle = brushStyle;
}

void ScribbleArea::slotSetBrush( QPixmap brushPix )
{
    _pixmap = brushPix;
    _brushStyle = Qt::TexturePattern;
}

void ScribbleArea::setShape( int shape )
{
    _shape = (Shape) shape;
}

void ScribbleArea::setPixmap( const QPixmap& pixmap )
{
    _shape = Image;
    _pixmap = pixmap;
}

void ScribbleArea::drawArrow( QPainter& painter, const QPoint& start, const QPoint& end )
{
    painter.drawLine( start, end );
    const double dx = end.x()-start.x();
    const double dy = end.y()-start.y();
    const double angle = atan2( dy, dx ) * 180 / M_PI ;

    painter.save();
    painter.translate(end);
    painter.rotate( angle );

    painter.save();
    painter.rotate( 20 );
    painter.drawLine( 0,0, -20, 0 );
    painter.restore();

    painter.rotate( -20 );
    painter.drawLine( 0,0, -20, 0 );

    painter.restore();
}


