#include "edit.h"
#include <QToolBar>
#include <QAction>
#include <QDocumentSelector>
#include <QMenu>
#include <QMenuBar>
#ifdef QTOPIA_PHONE
#include <QSoftMenuBar>
#endif
#include <QStackedWidget>
#include <QTextEdit>
#include <qtopia/qtopianamespace.h>
#include <QTextStream>

Edit::Edit( QWidget* parent, Qt::WFlags f) :QMainWindow( parent, f )
{
    QAction* newAc = new QAction( QIcon( ":/new" ), "New", this );
    QAction* loadAc = new QAction( QIcon( ":/pixmap" ), "Load", this );

    connect( loadAc, SIGNAL(activated()), this, SLOT(slotLoad()) );
    connect( newAc, SIGNAL(activated()), this, SLOT(slotNew()) );

#ifdef QTOPIA_PHONE
    QMenu* menu = QSoftMenuBar::menuFor( this );
    menu->addAction( newAc );
    menu->addAction( loadAc );
#else
    ToolBar *toolbar = new QToolBar(this);
    // We don't want the user to move toolbars arround.
    toolbar->setMovable( false );
    addToolBar( toolbar );
    toolbar->addAction( newAc );
    toolbar->addAction( loadAc );
#endif

    // Widget Stack, Editor Widget, and Document Selector.
    _stack = new QStackedWidget( this );
    setCentralWidget( _stack );

    _edit = new QTextEdit( _stack );
    _selector = new QDocumentSelector( "text/*", _stack );

    _stack->setCurrentWidget( _edit );
    connect( _selector, SIGNAL(documentSelected( const QContent& )), this, SLOT(slotLoad( const QContent& )));

    Qtopia::statusMessage("Welcome to PowerEdit");
}

void Edit::slotLoad()
{
    _stack->setCurrentWidget( _selector );
}

void Edit::slotNew()
{
    save();
    _edit->clear();
    _stack->setCurrentWidget( _edit );
}

void Edit::slotLoad( const QContent& content )
{
    if( !content.isValid() )
        return;

    save();
    QString txt;
    QFile file( content.file() );
    if( file.open( QIODevice::ReadOnly  ) ) {
        QTextStream ts( &file );
        txt = ts.readAll();
        file.close();
    }
    _edit->setPlainText( txt );
    _stack->setCurrentWidget( _edit );
    _doc = content;
}

void Edit::setDocument(const QString& name)
{
    save();
    slotLoad( QContent( name ) );
}

void Edit::closeEvent ( QCloseEvent * e )
{
    save();
    QMainWindow::closeEvent(e);

    // Need to get ready for starting again, if fast loading is checked.
    _edit->clear();
}

void Edit::save()
{
    if ( _doc.isValid() && _edit->document()->isModified() ) {
        _doc.setName( _edit->toPlainText().left( _edit->toPlainText().indexOf( " " ) ) );
        QFile file( _doc.file() );
        if( file.open( QIODevice::WriteOnly ) ) {
            QTextStream ts( &file );
            ts << _edit->toPlainText();
            _edit->document()->setModified( false );
            file.close();
        }
    }
}
