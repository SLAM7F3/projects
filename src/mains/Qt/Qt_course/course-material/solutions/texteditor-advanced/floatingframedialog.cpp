#include <QtGui>
#include "floatingframedialog.h"

FloatingFrameDialog::FloatingFrameDialog( QWidget* parent )
    :QDialog( parent )
{
    setupUi( this );

    type->insertItems( 0, QStringList() << "Variable" << "Fixed" << "Percentage" );
    positionEdit->insertItems( 0, QStringList() << "Inline" << "Float Left" << "Float Right" );
    connect( type, SIGNAL( activated( int ) ), this, SLOT( typeChanged( int ) ) );
    typeChanged(0);
}

void FloatingFrameDialog::typeChanged( int which )
{
    widthLabel->setEnabled( which != 0 );
    widthEdit->setEnabled( which != 0 );
    widthIndicator->setEnabled( which != 0 );

    widthIndicator->setText( which == 1 ? "pixels" : "%" );
    widthEdit->setMaximum( which == 1 ? 10000 : 100 );
}

QTextLength FloatingFrameDialog::length()
{
    QTextLength::Type tp = QTextLength::PercentageLength;
    if ( type->currentIndex() == 0 )
        tp = QTextLength::VariableLength;
    else if ( type->currentIndex() == 1 )
        tp = QTextLength::FixedLength;

    return QTextLength( tp, widthEdit->value() );
}

QTextFrameFormat::Position FloatingFrameDialog::position()
{
    if ( positionEdit->currentIndex() == 0 )
        return QTextFrameFormat::InFlow;
    else if ( positionEdit->currentIndex() == 1 )
        return QTextFrameFormat::FloatLeft;
    else
        return QTextFrameFormat::FloatRight;
}
