#include <qapplication.h>
#include <qsinterpreter.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qpushbutton.h>

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    // Read the script file
    QFile f("script");
    f.open( IO_ReadOnly );
    QTextStream stream(&f);
    QString script = stream.read();

    // Create the button and register it with the interpreter
    QPushButton* button = new QPushButton( 0, "button" );
    button->show();

    QSInterpreter* interpreter = QSInterpreter::defaultInterpreter();
    interpreter->addTransientObject( button );

    if ( !interpreter->checkSyntax( script ) )
        qDebug("Syntax Error: %s", interpreter->errorMessage().latin1());
    else
        interpreter->evaluate( script );


    return app.exec();
}
