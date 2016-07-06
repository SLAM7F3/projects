#include <qapplication.h>
#include <qlayout.h>
#include <qsproject.h>
#include <qseditor.h>
#include <qsscript.h>
#include <qpushbutton.h>
#include <qsinterpreter.h>

class Test :public QWidget
{
    Q_OBJECT
public:
    Test( QWidget* parent, const char* name = 0 ) :QWidget( parent, name ) {
        QString code = "function test() {\n\tprint(\"hello world\");\n}";

        _project = new QSProject( this );

        QSScript* script =  _project->createScript( "global", code );
        QVBoxLayout* lay1= new QVBoxLayout( this, 6 );
        QSEditor* editor = _project->createEditor( script, this );
        lay1->addWidget( editor );

        QHBoxLayout* lay2 = new QHBoxLayout( lay1, 6 );
        _revert = new QPushButton( "Revert script", this );
        lay2->addWidget( _revert );
        _revert->setEnabled( false );

        _reevaluate = new QPushButton( "Reevaluate script", this );
        lay2->addWidget( _reevaluate );
        _reevaluate->setEnabled( false );

        QPushButton* call = new QPushButton( "Call Function", this );
        lay2->addWidget( call );
        lay2->addStretch( 1 );

        connect( _revert, SIGNAL( clicked() ), this, SLOT( revert() ) );
        connect( _reevaluate, SIGNAL( clicked() ), this, SLOT(reevaluate() ) );
        connect( call, SIGNAL( clicked() ), this, SLOT( callTest() ) );
        connect( _project, SIGNAL( editorTextChanged() ), this, SLOT( dirty() ) );
    }

protected slots:
    void callTest() {
        _project->interpreter()->call( "test" );
    }

    void reevaluate() {
        _reevaluate->setEnabled( false );
        _revert->setEnabled( false );
        _project->commitEditorContents();
    }

    void dirty() {
        _revert->setEnabled( true );
        _reevaluate->setEnabled( true );
    }

    void revert() {
        _project->revertEditorContents();
        _revert->setEnabled( false );
        _reevaluate->setEnabled( false );
    }

private:
    QPushButton* _reevaluate;
    QPushButton* _revert;
    QSProject* _project;
};

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    Test* test = new Test(0);
    test->show();
    return app.exec();
}

#include "main.moc"
