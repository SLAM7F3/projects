#include <QtGui>
#include "testtool.h"

TestTool::TestTool( QWidget* parent )
    : QMainWindow( parent ), _maxId(0), _curId(0)
{
    // -------------------------------------------------- Actions
    // Quit
    QAction* quitAc = new QAction( QPixmap("icons/filequit.xpm"), tr("&Quit"), this );
    quitAc->setShortcut( Qt::CTRL+Qt::Key_Q );
    connect( quitAc, SIGNAL(triggered()), qApp, SLOT(quit()) );

    // Load
    QAction* openAc = new QAction( QPixmap("icons/fileopen.xpm"), tr("&Open"),this );
    connect( openAc, SIGNAL( triggered() ), this, SLOT(slotOpenFile()) );

    // -------------------------------------------------- Menu Bar
    QMenuBar* bar = menuBar();
    QMenu* fileMenu = new QMenu( tr("File"), this );

    bar->addMenu( fileMenu );
    fileMenu->addAction( openAc );
    fileMenu->addAction( quitAc );

    // -------------------------------------------------- The widgets

    _widgetStack = new QStackedWidget;

    // QFrame can be used for horizontal (and vertical) separators
    QFrame* hline =  new QFrame;
    hline->setFrameStyle( QFrame::HLine );
    hline->setFrameShadow( QFrame::Sunken );

    _backwardBut = new QPushButton( "<<" );
    connect( _backwardBut, SIGNAL(clicked()), this, SLOT(backward()));

    _forwardBut = new QPushButton( ">>" );
    connect( _forwardBut, SIGNAL(clicked()), this, SLOT(forward()));

    // -------------------------------------------------- Layouts
    QHBoxLayout* buts = new QHBoxLayout();
    buts->addWidget(_backwardBut);
    buts->addStretch(1);
    buts->addWidget(_forwardBut);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget( _widgetStack );
    layout->addWidget( hline );
    layout->addLayout( buts );

    QWidget *top = new QWidget( this );
    top->setLayout( layout );
    setCentralWidget( top );

    setState();
}

void TestTool::slotOpenFile()
{
    QString fileName = QFileDialog::getOpenFileName( this, QString(), QString(), "*.xml" );
    if ( fileName.isNull() )
        return;
    load( fileName );
}

void TestTool::load( QString fileName )
{
    QDomDocument doc;
    QFile f( fileName );
    if (! f.open( QIODevice::ReadOnly ) ) {
        qDebug("Couldn't open file!");
    }

    doc.setContent( &f );
    QDomElement top = doc.documentElement();

    if ( top.tagName() != "test-specification" ) {
        qWarning() << "XML error top tag was \"" << top.tagName() << "\"";
        return;
    }

    for ( QDomNode node = top.firstChild(); !node.isNull() ;
          node = node.nextSibling() ) {
        if ( node.isText() ) {
            qWarning() << "Syntax error: text ignore: \"" << node.toText().data() << "\"";
        }
        else if ( node.isComment() ) {
            qDebug() << "Skipping comment: \"" << node.toComment().data() << "\"";
        }
        else if ( node.isElement() ) {
            if ( node.toElement().tagName() == "test-case" ) {
                readTestCase( node.toElement() );
            }
            else {
                qDebug() << "Expected \"test-case\" got \"" <<  node.toElement().tagName() << "\"";
            }
        }
        else {
            qWarning("Unhandled node type");
        }
    }
    f.close();
    _widgetStack->setCurrentIndex(0);
    setState();
}

void TestTool::readTestCase( QDomElement top )
{
    QString name = top.attribute("name", "unnamed");
    QString objectives, input, output;
    for ( QDomNode node = top.firstChild(); !node.isNull() ;
          node = node.nextSibling() ) {
        if ( ! node.isElement() ) {
            qDebug("Syntax error");
        }
        else {
            QString tag = node.toElement().tagName();
            QString txt = node.toElement().text().trimmed();
            if ( tag == "objective" )
                objectives = txt;
            else if ( tag == "input" )
                input = txt;
            else if ( tag == "output" )
                output = txt;
            else
                qWarning() << "Unknow tag: \"" << tag << "\"";
        }
    }

    addPage(name, objectives, input, output);
}

void TestTool::addPage( const QString& name, const QString& objectives,
                        const QString& input, const QString& output )
{
    QLabel* nameLabel = new QLabel( "name: " );

    QLineEdit* nameEdit = new QLineEdit;
    nameEdit->setText( name );

    QLabel* objLabel = new QLabel( "Objectives:" );
    QTextEdit* objEdit = new QTextEdit;
    objEdit->setPlainText(objectives);

    QLabel* inputLabel = new QLabel( "Input:" );

    QTextEdit* inputEdit = new QTextEdit;
    inputEdit->setPlainText( input );

    QLabel* outputLabel = new QLabel( "Output:" );

    QTextEdit* outputEdit = new QTextEdit;
    outputEdit->setPlainText(output);

    // Layouts
    QHBoxLayout* nameLayout = new QHBoxLayout;
    nameLayout->addWidget( nameLabel );
    nameLayout->addWidget( nameEdit );

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addLayout( nameLayout );
    layout->addWidget( objLabel );
    layout->addWidget( objEdit );
    layout->addWidget( inputLabel );
    layout->addWidget( inputEdit );
    layout->addWidget( outputLabel );
    layout->addWidget( outputEdit );

    QWidget* top = new QWidget;
    top->setLayout( layout );

    _widgetStack->insertWidget( nextId(), top );
}

void TestTool::forward()
{
    _widgetStack->setCurrentIndex( ++_curId );
    setState();
}

void TestTool::backward()
{
    _widgetStack->setCurrentIndex( --_curId );
    setState();
}

void TestTool::setState()
{
    _forwardBut->setEnabled( _curId < _maxId-1 );
    _backwardBut->setEnabled( _curId > 0 );

}
