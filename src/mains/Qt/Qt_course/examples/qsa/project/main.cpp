#include <qapplication.h>
#include <qsinterpreter.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qpushbutton.h>
#include <qsproject.h>
#include <qsscript.h>
#include <qsworkbench.h>
#include <qsscript.h>

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    // Read the script file
    QFile f("button");
    f.open( IO_ReadOnly );
    QTextStream stream(&f);
    QString str = stream.read();

    // Create the button and register it with the interpreter
    QPushButton* button = new QPushButton( 0, "button" );
    button->show();

    QSProject* project = new QSProject( 0 );
    project->load("project.prj");

    qDebug("%p", project->interpreter()->currentContext());

    project->addObject( button );

    QSWorkbench* wb = new QSWorkbench( project );
    wb->open();
    project->addSignalHandler( button, SIGNAL( clicked() ), "f" );


    qApp->setMainWidget( button );

    int ret = app.exec();
    project->save();
    return ret;
}
