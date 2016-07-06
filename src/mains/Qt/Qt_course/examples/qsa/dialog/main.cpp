#include <qapplication.h>
#include <qsinterpreter.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qpushbutton.h>
#include <qsinputdialogfactory.h>

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    // Read the script file
    QFile f("script");
    f.open( IO_ReadOnly );
    QTextStream stream(&f);
    QString script = stream.read();

    QSInterpreter* interpreter = QSInterpreter::defaultInterpreter();
    interpreter->addObjectFactory( new QSInputDialogFactory );

    if ( !interpreter->checkSyntax( script ) )
        qDebug("Syntax Error: %s", interpreter->errorMessage().latin1());
    else
        interpreter->evaluate( script );


    return app.exec();
}
