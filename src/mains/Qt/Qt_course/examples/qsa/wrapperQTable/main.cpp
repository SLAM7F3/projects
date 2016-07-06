#include <qapplication.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qsinterpreter.h>
#include <qswrapperfactory.h>
#include <qtable.h>

class TableWrapper :public QObject {
    Q_OBJECT

public:
    TableWrapper( QTable* table ) : _table( table ) {}

public slots:
    // With this wrapper, scripts can now access the method QTable::setText.
    void setText( int row, int col, const QString& txt ) {
        _table->setText( row, col, txt );
    }

private:
    QTable* _table;
};


class TableFactory :public QSWrapperFactory {
public:
    TableFactory() {
        registerWrapper( "QTable" );
    }

    virtual QObject * create ( const QString & className, void * ptr ) {
        if ( className == QString::fromLatin1( "QTable" ) ) {
            return new TableWrapper( static_cast<QTable*>( ptr ) );
        }
        return 0;
    }
};


int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    QTable* table = new QTable( 0, "table");
    table->show();

    // Read the script file
    QFile f("script");
    f.open( IO_ReadOnly );
    QTextStream stream(&f);
    QString script = stream.read();

    // Register the Table
    QSInterpreter* interpreter = QSInterpreter::defaultInterpreter();
    interpreter->addWrapperFactory( new TableFactory );
    interpreter->addTransientObject( table );

    if ( !interpreter->checkSyntax( script ) )
        qDebug("Syntax Error: %s", interpreter->errorMessage().latin1());
    else
        interpreter->evaluate( script );



    return app.exec();
}

#include "main.moc"
