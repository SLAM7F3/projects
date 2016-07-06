#include <qapplication.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qpushbutton.h>
#include <qvbox.h>

#include <qsinterpreter.h>
#include <qsobjectfactory.h>

class CompanyButton :public QPushButton
{
    Q_OBJECT
    Q_ENUMS( Type );
public:
    CompanyButton( QWidget* parent, const char* name ) :QPushButton( parent, name )
        {
        }

    enum Type { KDAB = 1, TT = 2 };

public slots:

    void setEnum( int type )
        {
            if ( type == KDAB )
                setText( "Klaralvdalens Datakonsult AB" );
            else if ( type == TT )
                setText( "Trolltech" );
            else
                setText( "Undefined" );
            resize( sizeHint() );
        }
};

class ButtonFacotory :public QSObjectFactory {

public:
    ButtonFacotory()
        {
            registerClass( "CompanyButton", new CompanyButton(0, "hidden button") );
        }

    virtual QObject * create ( const QString& , const QSArgumentList&, QObject* )
        {
            throwError("You can not create instances of this class");
            return 0;
        }

};



int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    // Read the script file
    QFile f("script");
    f.open( IO_ReadOnly );
    QTextStream stream(&f);
    QString script = stream.read();

    // Create the button and register it with the interpreter
    QVBox* top = new QVBox( 0, "companies" );
    new CompanyButton( top, "button1" );
    new CompanyButton( top, "button2" );
    top->show();

    QSInterpreter* interpreter = QSInterpreter::defaultInterpreter();

    // This is to register the CompanyButton class.
    interpreter->addObjectFactory( new ButtonFacotory() );

    interpreter->addTransientObject( top );

    if ( !interpreter->checkSyntax( script ) )
        qDebug("Syntax Error: %s", interpreter->errorMessage().latin1());
    else
        interpreter->evaluate( script );


    return app.exec();
}

#include "main.moc"

