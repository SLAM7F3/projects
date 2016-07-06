#include "compass2.h"

#include <QtGui>

static const char* s_directionName[] = { "North", "NorthWest", "West", "SouthWest", "South", "SouthEast", "East", "NorthEast" };

CompassWidget2::CompassWidget2( CompassDirection d, QWidget* parent ) :
	QWidget( parent )
{
	init();
	setDirection( d );
}


CompassWidget2::CompassWidget2( QWidget* parent ) :
	QWidget( parent )
{
	init();
	setDirection( North );
}


CompassWidget2::~CompassWidget2()
{
	for ( int d = North; d < NumDirections; ++d )
		delete _region[d];
}


void CompassWidget2::init()
{
        setFocusPolicy( Qt::ClickFocus );
	for ( int d = North; d < NumDirections; ++d ) {
		_region[d] = 0;
		_points[d].resize( 5 );
	}
}


void CompassWidget2::setDirection( CompassDirection d )
{
	_direction = d;
	update();
}


CompassWidget2::CompassDirection CompassWidget2::direction() const
{
	return _direction;
}


void CompassWidget2::paintEvent( QPaintEvent* )
{
	QPainter p( this );
	p.setPen( Qt::black );

	QBrush fillBrush( palette().color( QPalette::Dark) );

	for ( int d = North; d < NumDirections; ++d )
	{
		p.save();
		if( _direction == d )
			p.setBrush( fillBrush );
		p.drawPolygon( _points[d] );
		p.restore();
	}
}


void CompassWidget2::resizeEvent( QResizeEvent* )
{
	int width_1_8 = width() / 8;
	int width_3_16 = width() * 3 / 16;
	int width_3_8 = width() * 3 / 8;
	int width_7_16 = width() * 7 / 16;
	int width_1_2 = width() / 2;
	int width_9_16 = width() * 9 / 16;
	int width_5_8 = width() * 5 / 8;
	int width_13_16 = width() * 13 / 16;
	int width_7_8 = width() * 7 / 8;
	int height_1_8 = height() / 8;
	int height_3_16 = height() * 3 / 16;
	int height_3_8 = height() * 3 / 8;
	int height_7_16 = height() * 7 / 16;
	int height_1_2 = height() / 2;
	int height_9_16 = height() * 9 / 16;
	int height_5_8 = height() * 5 / 8;
	int height_13_16 = height() * 13 / 16;
	int height_7_8 = height() * 7 / 8;

	_points[North].setPoint( 0, width_1_2, 0 );
	_points[North].setPoint( 1, width_3_8, height_3_8 );
	_points[North].setPoint( 2, width_1_2, height_1_2 );
	_points[North].setPoint( 3, width_5_8, height_3_8 );
	_points[North].setPoint( 4, width_1_2, 0 );

	_points[NorthWest].setPoint( 0, width_1_8, height_1_8 );
	_points[NorthWest].setPoint( 1, width_3_16, height_7_16 );
	_points[NorthWest].setPoint( 2, width_3_8, height_3_8 );
	_points[NorthWest].setPoint( 3, width_7_16, height_3_16 );
	_points[NorthWest].setPoint( 4, width_1_8, height_1_8 );

	_points[NorthEast].setPoint( 0, width_7_8, height_1_8 );
	_points[NorthEast].setPoint( 1, width_13_16, height_7_16 );
	_points[NorthEast].setPoint( 2, width_5_8, height_3_8 );
	_points[NorthEast].setPoint( 3, width_9_16, height_3_16 );
	_points[NorthEast].setPoint( 4, width_7_8, height_1_8 );

	_points[West].setPoint( 0, 0, height_1_2 );
	_points[West].setPoint( 1, width_3_8, height_3_8 );
	_points[West].setPoint( 2, width_1_2, height_1_2 );
	_points[West].setPoint( 3, width_3_8, height_5_8 );
	_points[West].setPoint( 4, 0, height_1_2 );

	_points[East].setPoint( 0, width(), height_1_2 );
	_points[East].setPoint( 1, width_5_8, height_3_8 );
	_points[East].setPoint( 2, width_1_2, height_1_2 );
	_points[East].setPoint( 3, width_5_8, height_5_8 );
	_points[East].setPoint( 4, width(), height_1_2 );

	_points[SouthWest].setPoint( 0, width_1_8, height_7_8 );
	_points[SouthWest].setPoint( 1, width_3_16, height_9_16 );
	_points[SouthWest].setPoint( 2, width_3_8, height_5_8 );
	_points[SouthWest].setPoint( 3, width_7_16, height_13_16 );
	_points[SouthWest].setPoint( 4, width_1_8, height_7_8 );

	_points[SouthEast].setPoint( 0, width_7_8, height_7_8 );
	_points[SouthEast].setPoint( 1, width_13_16, height_9_16 );
	_points[SouthEast].setPoint( 2, width_5_8, height_5_8 );
	_points[SouthEast].setPoint( 3, width_9_16, height_13_16 );
	_points[SouthEast].setPoint( 4, width_7_8, height_7_8 );

	_points[South].setPoint( 0, width_1_2, height() );
	_points[South].setPoint( 1, width_3_8, height_5_8 );
	_points[South].setPoint( 2, width_1_2, height_1_2 );
	_points[South].setPoint( 3, width_5_8, height_5_8 );
	_points[South].setPoint( 4, width_1_2, height() );

	for ( int d = North; d < NumDirections; ++d )
	{
		delete _region[d];
		_region[d] = new QRegion( _points[d] );
	}

}

void CompassWidget2::mousePressEvent( QMouseEvent* e )
{
	// Hit test
	for ( int d = North; d < NumDirections; ++d ) {
		if( _region[d]->contains( e->pos() ) ) {
			setDirectionAndEmit( static_cast<CompassDirection>( d ) );
			update();
			break;
		}
	}
}

void CompassWidget2::keyPressEvent( QKeyEvent* ev )
{
	switch ( ev->key() ) {
        case Qt::Key_Right:
		if ( _direction == North )
			setDirectionAndEmit( NorthEast );
		else
			setDirectionAndEmit( static_cast<CompassDirection>( _direction - 1 ) );
		break;
        case Qt::Key_Left:
		if ( _direction == NorthEast )
			setDirectionAndEmit( North );
		else
			setDirectionAndEmit( static_cast<CompassDirection>( _direction + 1 ) );
		break;
        default:
		break;
	}
}

void CompassWidget2::setDirectionAndEmit( CompassDirection d )
{
	setDirection( d );
	emit directionChanged( d );
	emit directionChanged( s_directionName[d] );
}
