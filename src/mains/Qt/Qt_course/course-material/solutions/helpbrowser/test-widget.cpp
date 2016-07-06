#include <QtGui>
#include "test-widget.h"
#include "helpbrowser.h"


TestWidget::TestWidget( QWidget* parent )
    :QWidget( parent )
{
    _helpBrowser = new HelpBrowser( this );
    _helpBrowser->setHelpPath( "help-text/" );

    QCheckBox* box1 = new QCheckBox( "Check box 1" );
    _helpBrowser->setHelp( box1, "index.html#box1" );

    QCheckBox* box2 = new QCheckBox( "Check box 2" );
    _helpBrowser->setHelp( box2, "index.html#box2" );

    QCheckBox* box3 = new QCheckBox( "Check box 3" );
    _helpBrowser->setHelp( box3, "box3.html" );

    QCheckBox* box4 = new QCheckBox( "Check box 4" );

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget( box1 );
    layout->addWidget( box2 );
    layout->addWidget( box3 );
    layout->addWidget( box4 );
    setLayout( layout );
}
