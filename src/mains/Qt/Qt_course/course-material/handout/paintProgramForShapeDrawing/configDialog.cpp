#include <QtGui>
#include "configDialog.h"

ConfigDialog::ConfigDialog( QWidget* parent )
    : QDialog( parent )
{
    // Line Width and line Style

    QLabel* labelWidth = new QLabel( "Width" );
    _lineWidth = new QSpinBox;
    _lineWidth->setRange( 1, 10 );

    QLabel* labelStyle = new QLabel( "Style" );
    _lineStyle = new QComboBox;
    _lineStyle->addItem( QPixmap("icons/solid.xpm"), "Solid" );
    _lineStyle->addItem( QPixmap("icons/dash.xpm"), "Dash" );
    _lineStyle->addItem( QPixmap("icons/dot.xpm"), "Dot" );
    _lineStyle->addItem( QPixmap("icons/dashdot.xpm"), "DashDot" );
    _lineStyle->addItem( QPixmap("icons/dashdotdot.xpm"), "DashDotDot" );
    _lineStyle->addItem( "None" );

    QGroupBox* penBox = new QGroupBox( "Pen" );

    // Brush
    QLabel* brushTypeLabel = new QLabel( "Brush Style" );
    _brushType = new QComboBox;
    _brushType->addItems( QStringList() << "No Brush" << "Pattern" << "Pixmap" << "Gradient" );

    _styleLabel = new QLabel( "Style" );
    _brushStyle = new QComboBox;
    QStringList items;
    items << "Solid" <<	"Dense1" << "Dense2"
          << "Dense3" << "Dense4" << "Dense5"
          << "Dense6" << "Dense7" << "Horizontal"
          << "Verical" << "Cross" << "BDiag"
          << "FDiag" << "DiagCross";
    _brushStyle->insertItems( 0, items );

    _pixmapLabel = new QLabel( "Pixmap" );
    _pixmap = new QLineEdit;

    _gradientLabel = new QLabel( "Gradient Type" );
    _gradient = new QComboBox;
    _gradient->insertItems( 0, QStringList() << "Linear" << "Conical" << "Radial" );

    connect( _brushType, SIGNAL( activated( int ) ), this, SLOT( slotBrushTypeChanged( int ) ) );
    slotBrushTypeChanged( _brushType->currentIndex() );

    QGroupBox* brushBox = new QGroupBox( "Brush" );

    // OK, Cancel
    QPushButton* apply = new QPushButton( "Apply" );
    QPushButton* ok = new QPushButton( "OK" );
    QPushButton* cancel = new QPushButton( "Cancel" );

    connect( ok, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( cancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( ok, SIGNAL( clicked() ), this, SLOT( slotEmitData() ) );
    connect( apply, SIGNAL( clicked() ), this, SLOT( slotEmitData() ) );

    // Layouts
    QGridLayout* gridLayout = new QGridLayout;
    gridLayout->addWidget( labelWidth, 0, 0 );
    gridLayout->addWidget( _lineWidth, 0, 1 );
    gridLayout->addWidget( labelStyle, 1, 0 );
    gridLayout->addWidget( _lineStyle, 1, 1 );
    gridLayout->setColumnStretch( 0, 0 );
    gridLayout->setColumnStretch( 1, 0 );
    gridLayout->setColumnStretch( 2, 1 );
    penBox->setLayout( gridLayout );

    gridLayout = new QGridLayout;
    gridLayout->addWidget( brushTypeLabel, 0, 0 );
    gridLayout->addWidget( _brushType, 0, 1 );
    gridLayout->addWidget( _styleLabel, 1, 0 );
    gridLayout->addWidget( _brushStyle, 1, 1 );
    gridLayout->addWidget( _pixmapLabel, 2, 0 );
    gridLayout->addWidget( _pixmap, 2, 1 );
    gridLayout->addWidget( _gradientLabel, 3, 0 );
    gridLayout->addWidget( _gradient, 3, 1 );
    brushBox->setLayout( gridLayout );

    QHBoxLayout* lay = new QHBoxLayout;
    lay->addStretch( 1 );
    lay->addWidget( apply );
    lay->addWidget( ok );
    lay->addWidget( cancel );


    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->addWidget( penBox );
    layout->addWidget( brushBox );
    layout->addStretch( 1 );
    layout->addLayout( lay );



}

void ConfigDialog::slotBrushTypeChanged( int which )
{
    _styleLabel->setEnabled( which == 1 );
    _brushStyle->setEnabled( which == 1 );
    _pixmapLabel->setEnabled( which == 2 );
    _pixmap->setEnabled( which == 2 );
    _gradientLabel->setEnabled( which == 3 );
    _gradient->setEnabled( which == 3 );
}



void ConfigDialog::slotEmitData()
{
    Qt::PenStyle penStyle = Qt::NoPen;
    switch ( _lineStyle->currentIndex() ) {
    case 0: penStyle = Qt::SolidLine; break;
    case 1: penStyle = Qt::DashLine; break;
    case 2: penStyle = Qt::DotLine; break;
    case 3: penStyle = Qt::DashDotLine; break;
    case 4: penStyle = Qt::DashDotDotLine; break;
    case 5: penStyle = Qt::NoPen; break;
    }
    emit setPen( _lineWidth->text().toInt(), penStyle );

    switch ( _brushType->currentIndex() ) {
    case 0:
        emit setBrush(Qt::NoBrush);
        break;
    case 1:
    {
        Qt::BrushStyle brushStyle = Qt::NoBrush;
        QString type = _brushStyle->currentText();
        if ( type ==  "Solid" )
            brushStyle = Qt::SolidPattern;
        else if ( type == "Dense1" )
            brushStyle = Qt::Dense1Pattern;
        else if ( type == "Dense2" )
            brushStyle = Qt::Dense2Pattern;
        else if ( type == "Dense3" )
            brushStyle = Qt::Dense3Pattern;
        else if ( type == "Dense4" )
            brushStyle = Qt::Dense4Pattern;
        else if ( type == "Dense5" )
            brushStyle = Qt::Dense5Pattern;
        else if ( type == "Dense6" )
            brushStyle = Qt::Dense6Pattern;
        else if ( type == "Dense7" )
            brushStyle = Qt::Dense7Pattern;
        else if ( type == "Horizontal" )
            brushStyle = Qt::HorPattern;
        else if ( type == "Verical" )
            brushStyle = Qt::VerPattern;
        else if ( type == "Cross" )
            brushStyle = Qt::CrossPattern;
        else if ( type == "BDiag" )
            brushStyle = Qt::BDiagPattern;
        else if ( type == "FDiag" )
            brushStyle = Qt::FDiagPattern;
        else if ( type == "DiagCross" )
            brushStyle = Qt::DiagCrossPattern;
        else if ( type == "Custom" )
            brushStyle = Qt::TexturePattern;
        emit setBrush( brushStyle );
        break;
    }

    case 2:
        emit setBrush(  QPixmap( _pixmap->text() ) );
        break;

    case 3:
        switch ( _gradient->currentIndex() ) {
        case 0: emit setBrush( Qt::LinearGradientPattern ); break;
        case 1: emit setBrush( Qt::ConicalGradientPattern ); break;
        case 2: emit setBrush( Qt::RadialGradientPattern ); break;
        }
        break;
    }
}
