#include <QtGui>

#include "puzzle.h"
Puzzle::Puzzle( QWidget* parent )
    :QWidget( parent ), _count(0)
{
    _image = QPixmap( "Images/jesper.jpg" );
    _question = QPixmap( "Images/question.png" );

    QPolygon bodyPoints, eyePoints;

    bodyPoints << QPoint( 50, 180 ) << QPoint( 310, 260 ) << QPoint( 420, 330 ) << QPoint( 520, 560 ) << QPoint( 360 , 800 ) <<
        QPoint( 200, 800 ) << QPoint( 50, 400 );

    eyePoints << QPoint( 360, 220 ) << QPoint( 430, 225) << QPoint( 420, 255 )<< QPoint( 350, 240 );

    _regions
        << QRegion( QRect( QPoint(50, 150), QPoint( 170, 300 ) ), QRegion::Ellipse )
        << QRect( QPoint( 180, 350), QPoint( 380, 650) )
        << bodyPoints
        << eyePoints
        << QRect( QPoint( 340, 100 ), QPoint( 460, 180 ) )
        << QRect( 0,0, _image.width(), _image.height() );

    setFixedSize( _image.size() );
}

void Puzzle::paintEvent( QPaintEvent* )
{
    QPainter painter( this );
    painter.drawPixmap( 0,0, _question );
    QRegion region;
    for ( int i = 0; i < _count; ++i )
        region += _regions[i];
    painter.setClipRegion( region );
    painter.drawPixmap( 0,0, _image );
}

void Puzzle::showNext()
{
    _count = (_count + 1) % (_regions.size()+1);
    repaint();
}

