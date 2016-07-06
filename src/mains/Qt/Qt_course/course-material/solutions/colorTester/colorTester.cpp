#include <QtGui>
#include "colorTester.h"

ColorTester::ColorTester( QWidget* parent ) : QWidget( parent )
{
    QLabel* label = new QLabel("Color is: " );
    _colorLabel = new QLabel( "Not Set" );
    _colorLabel->setAutoFillBackground( true );
    QPushButton* button = new QPushButton( "Select Color" );
    connect( button, SIGNAL( clicked() ), this, SLOT(slotSelectColor()) );

    QHBoxLayout* hlay = new QHBoxLayout;
    hlay->addWidget( label );
    hlay->addWidget( _colorLabel );

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addLayout( hlay );
    layout->addWidget( button );

    setLayout( layout );

}

void ColorTester::slotSelectColor()
{
    QColor color = QColorDialog::getColor( Qt::black, this );
    if ( color.isValid() )
    {
        _colorLabel->setText( color.name() );

    	QPalette pal = _colorLabel->palette();
    	pal.setColor( QPalette::Window, color );
    	_colorLabel->setPalette( pal );
    }
}
