#include <qapplication.h>
#include "file.h"
#include <qsobjectfactory.h>
#include <qsinterpreter.h>

class FileFactory :public QSObjectFactory {
public:
    FileFactory() {
        registerClass( "File", new File );
    }
    virtual QObject* create( const QString & className,
                             const QSArgumentList& args,
                             QObject*  ) {
        if ( className == "File") {
            if ( args.count() == 0 )
                return new File();
            else if ( args.count() == 1 ) {
                QSArgument arg = args[0];
                if ( arg.type() != QSArgument::Variant || arg.variant().type() != QVariant::String ) {
                    throwError( "Invalid type for argument 1 to File, must be a string" );
                    return 0;
                }
                return new File( arg.variant().toString() );
            }
            else {
                throwError( "Wrong number of arguments to File" );
                return 0;
            }
        }
        else {
            Q_ASSERT( false );
            return 0;
        }
    }

};



int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    // Read the script file
    QFile f("script");
    f.open( IO_ReadOnly );
    QTextStream stream(&f);
    QString script = stream.read();

    QSInterpreter* interpreter = QSInterpreter::defaultInterpreter();
    interpreter->addObjectFactory( new FileFactory );

    if ( !interpreter->checkSyntax( script ) )
        qDebug("Syntax Error: %s", interpreter->errorMessage().latin1());
    else
        interpreter->evaluate( script );

}
