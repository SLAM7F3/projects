#include <qapplication.h>
#include <qsinterpreter.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qlineedit.h>
#include <qvbox.h>

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    // Read the script file
    QFile f("script");
    f.open( IO_ReadOnly );
    QTextStream stream(&f);
    QString script = stream.read();

    // Create the listbox and register it with the interpreter
    QVBox* top = new QVBox( 0, "top" );
    new QLineEdit( top, "edit1" );
    new QLineEdit( top, "edit2" );
    top->show();

    QSInterpreter* interpreter = QSInterpreter::defaultInterpreter();
    interpreter->addTransientObject( top );

    if ( !interpreter->checkSyntax( script ) )
        qDebug("Syntax Error: %s", interpreter->errorMessage().latin1());
    else
        interpreter->evaluate( script );


    return app.exec();
}
