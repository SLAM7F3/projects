#include "compass.h"

#include <QtGui>

CompassWidget::CompassWidget( CompassDirection d, QWidget* parent ) :
	QWidget( parent )
{
	init();
	setDirection( d );
}


CompassWidget::CompassWidget( QWidget* parent ) :
	QWidget( parent )
{
	init();
	setDirection( North );
}


void CompassWidget::init()
{
	_buttongroup = new QButtonGroup( this );
	_buttongroup->setExclusive( true );
	connect( _buttongroup, SIGNAL( buttonClicked( QAbstractButton* ) ),
             this, SLOT( slotButtonChecked( QAbstractButton* ) ) );

	QGridLayout* layout = new QGridLayout( this );

	// row 1
	_northbutton = new QPushButton( tr( "N" ), this );
    _northbutton->setCheckable( true );
	layout->addWidget( _northbutton, 0, 2 );
	_buttongroup->addButton( _northbutton );

	// row 2
	_northwestbutton = new QPushButton( tr( "NW" ), this );
    _northwestbutton->setCheckable( true );
	layout->addWidget( _northwestbutton, 1, 1 );
	_buttongroup->addButton( _northwestbutton );
	_northeastbutton = new QPushButton( tr( "NE" ), this );
    _northeastbutton->setCheckable( true );
	layout->addWidget( _northeastbutton, 1, 3 );
	_buttongroup->addButton( _northeastbutton );

	// row 3
	_westbutton = new QPushButton( tr( "W" ), this );
    _westbutton->setCheckable( true );
	layout->addWidget( _westbutton, 2, 0 );
	_buttongroup->addButton( _westbutton );
	_eastbutton = new QPushButton( tr( "E" ), this );
    _eastbutton->setCheckable( true );
	layout->addWidget( _eastbutton, 2, 4 );
	_buttongroup->addButton( _eastbutton );

	// row 4
	_southwestbutton = new QPushButton( tr( "SW" ), this );
    _southwestbutton->setCheckable( true );
	layout->addWidget( _southwestbutton, 3, 1 );
	_buttongroup->addButton( _southwestbutton );
	_southeastbutton = new QPushButton( tr( "SE" ), this );
    _southeastbutton->setCheckable( true );
	layout->addWidget( _southeastbutton, 3, 3 );
	_buttongroup->addButton( _southeastbutton );

	// row 5
	_southbutton = new QPushButton( tr( "S" ), this );
    _southbutton->setCheckable( true );
	layout->addWidget( _southbutton, 4, 2 );
	_buttongroup->addButton( _southbutton );
}


void CompassWidget::setDirection( CompassDirection d )
{
	_direction = d;

    switch( d ) {
    case North:
        _northbutton->setChecked( true );
        break;
    case NorthWest:
        _northwestbutton->setChecked( true );
        break;
    case NorthEast:
        _northeastbutton->setChecked( true );
        break;
    case East:
        _eastbutton->setChecked( true );
        break;
    case West:
        _westbutton->setChecked( true );
        break;
    case SouthWest:
        _southwestbutton->setChecked( true );
        break;
    case SouthEast:
        _southeastbutton->setChecked( true );
        break;
    case South:
        _southbutton->setChecked( true );
        break;
    }
}


CompassWidget::CompassDirection CompassWidget::direction() const
{
	return _direction;
}


void CompassWidget::slotButtonChecked( QAbstractButton* button )
{
    if( button == _northbutton ) {
        emit directionChanged( North );
        emit directionChanged( "North" );
    } else if( button == _northwestbutton ) {
        emit directionChanged( NorthWest );
        emit directionChanged( "NorthWest" );
    } else if( button == _northeastbutton ) {
        emit directionChanged( NorthEast );
        emit directionChanged( "NorthEast" );
    } else if( button == _westbutton ) {
        emit directionChanged( West );
        emit directionChanged( "West" );
    } else if( button == _eastbutton ) {
        emit directionChanged( East );
        emit directionChanged( "East" );
    } else if( button == _southwestbutton ) {
        emit directionChanged( SouthWest );
        emit directionChanged( "SouthWest" );
    } else if( button == _southeastbutton ) {
        emit directionChanged( SouthEast );
        emit directionChanged( "SouthEast" );
    } else if( button == _southbutton ) {
        emit directionChanged( South );
        emit directionChanged( "South" );
    }
}

