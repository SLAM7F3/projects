#include<QtGui>

class IPValidator :public QValidator
{
public:
    IPValidator( QLineEdit* parent ) :QValidator( parent ) {}
protected:
    virtual State validate ( QString& input, int& ) const {
        for ( int pos = 0; pos<15;pos+=4 ) {
            bool ok1;
            int i = input.mid(pos,1).toInt(&ok1);
            bool ok2;
            int j = input.mid(pos+1,1).toInt(&ok2);
            bool ok3;
            int k = input.mid(pos+2,1).toInt(&ok3);

            if ( ( ok1 && i > 2 ) ||
                 ( ok1 && ok2 && i == 2 && j > 5 ) ||
                 (ok1 && ok2 && ok3 && i*100+j*10+k > 255 ) )
                return Invalid;
        }
        return Acceptable;
    }
};

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    QLabel* ipLabel = new QLabel( "IP Address:" );
    QLineEdit* ipEdit = new QLineEdit;
    ipEdit->setInputMask("000.000.000.000;_");
    ipEdit->setValidator( new IPValidator( ipEdit ) );

    QLabel* isoLabel = new QLabel("ISO Date:" );
    QLineEdit* isoEdit = new QLineEdit;
    isoEdit->setInputMask( "0000-00-00" );

    QLabel* licenseLabel = new QLabel("License Number:" );
    QLineEdit* licenseEdit = new QLineEdit;
    licenseEdit->setInputMask( ">AAAAA-AAAAA-AAAAA-AAAAA-AAAAA;#" );

    QWidget* top = new QWidget;
    QGridLayout* layout = new QGridLayout( top );
    layout->addWidget( ipLabel, 0, 0 );
    layout->addWidget( ipEdit, 0, 1 );
    layout->addWidget( isoLabel, 1, 0 );
    layout->addWidget( isoEdit, 1, 1 );
    layout->addWidget( licenseLabel, 2, 0 );
    layout->addWidget( licenseEdit, 2, 1 );

    top->show();
    return app.exec();
}
