#include <QtGui>

class TestWidget : public QWidget
{
public:
    TestWidget( QWidget* parent );
private:
    QCheckBox* checkBox1;
    QCheckBox* checkBox2;
    QRadioButton* radioButton1;
    QRadioButton* radioButton2;
    QRadioButton* radioButton3;
    QComboBox* comboBox;
    QSpinBox* spinBox;
    QPushButton* okButton;
};


TestWidget::TestWidget( QWidget* parent )
    : QWidget( parent )
{
    QVBoxLayout* layout = new QVBoxLayout( this );

    layout->setMargin( 10 );
    layout->setSpacing( 10 );
    checkBox1 = new QCheckBox( this );
    checkBox1->setText( tr( "Checkbox" ) );
    layout->addWidget( checkBox1 );

    checkBox2 = new QCheckBox( this );
    checkBox2->setText( tr( "Disabled checkbox" ) );
    checkBox2->setEnabled( false ); // don't forget to test disabled widgets
    layout->addWidget( checkBox2 );

    radioButton1 = new QRadioButton( this );
    radioButton1->setText( tr( "Choice 1" ) );
    layout->addWidget( radioButton1 );

    radioButton2 = new QRadioButton( this );
    radioButton2->setText( tr( "Choice 2" ) );
    layout->addWidget( radioButton2 );

    radioButton3 = new QRadioButton( this );
    radioButton3->setText( tr( "Disabled radiobutton" ) );
    radioButton3->setEnabled( false ); // don't forget to test disabled widgets
    layout->addWidget( radioButton3 );

    comboBox = new QComboBox( this );
    comboBox->addItem( tr( "First Item" ) );
    comboBox->addItem( tr( "Second Item" ) );
    comboBox->addItem( tr( "Third Item" ) );
    layout->addWidget( comboBox );

    spinBox = new QSpinBox( this );
    spinBox->setMinimum( 1 );
    spinBox->setMaximum( 10 );
    spinBox->setValue( 3 );
    layout->addWidget( spinBox );

    okButton = new QPushButton( tr( "&OK" ), this );
    connect( okButton, SIGNAL( clicked() ), SLOT( close() ) );
    layout->addWidget( okButton );
}

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    TestWidget* box = new TestWidget( 0 );
    box->show();

    QObject::connect( qApp, SIGNAL( lastWindowClosed() ), qApp, SLOT( quit() ) );
    return app.exec();
}
