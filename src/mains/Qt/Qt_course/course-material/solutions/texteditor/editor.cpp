#include <QtGui>
#include "editor.h"

Editor::Editor( QWidget* parent ) : QMainWindow( parent )
{
    // -------------------------------------------------- Actions

    // Open
    QAction* fileOpenAction = new QAction( this );
    fileOpenAction->setText( "&Open" );
    fileOpenAction->setToolTip( "Open a file for editing" );
    fileOpenAction->setIcon( QIcon( QPixmap( "fileopen.png" ) ) );
    connect( fileOpenAction, SIGNAL( triggered() ), this, SLOT( slotFileOpen() ) );

    // Save
    _fileSaveAction = new QAction( this );
    _fileSaveAction->setText( "&Save" );
    _fileSaveAction->setToolTip( "Save current file" );
    _fileSaveAction->setIcon( QIcon( QPixmap( "filesave.png" ) ) );
    connect( _fileSaveAction, SIGNAL( triggered() ), this, SLOT( slotFileSave() ) );

    // Print
    _filePrintAction = new QAction( this );
    _filePrintAction->setText( "&Print" );
    _filePrintAction->setToolTip( "Print current file" );
    _filePrintAction->setIcon( QIcon( QPixmap( "fileprint.png" ) ) );
    connect( _filePrintAction, SIGNAL( triggered() ), this, SLOT( slotFilePrint() ) );

    // Quit
    QAction* quitAction = new QAction( this );
    quitAction->setText( "&Quit" );
    quitAction->setToolTip( "Quit the text editor" );
    connect( quitAction, SIGNAL( triggered() ), this, SLOT( slotQuit() ) );

    // About
    QAction* aboutAction = new QAction( this );
    aboutAction->setText( "About" );
    aboutAction->setToolTip( "About this application" );
    connect( aboutAction, SIGNAL( triggered() ), this, SLOT( slotAbout() ) );

    // About Qt
    QAction* aboutQtAction = new QAction( this );
    aboutQtAction->setText( "About Qt" );
    aboutQtAction->setToolTip( "About the Qt framework" );
    connect( aboutQtAction, SIGNAL( triggered() ), this, SLOT( slotAboutQt() ) );

    // -------------------------------------------------- Menubar
    QMenu* fileMenu = new QMenu( "&File", menuBar() );
    menuBar()->addMenu( fileMenu );

    fileMenu->addAction(fileOpenAction);
    fileMenu->addAction(_fileSaveAction);
    fileMenu->addAction(_filePrintAction);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAction);

    QMenu* helpMenu = new QMenu( "&Help", menuBar() );
    menuBar()->addMenu( helpMenu );

    helpMenu->addAction(aboutAction);
    helpMenu->addAction(aboutQtAction);

    // -------------------------------------------------- Toolbar
    QToolBar* toolbar = new QToolBar( "Main Toolbar", this );
    addToolBar( toolbar );

    toolbar->addAction(fileOpenAction);
    toolbar->addAction(_fileSaveAction);
    toolbar->addAction(_filePrintAction);


    // -------------------------------------------------- Statusbar, Editor
    _fileNameLabel = new QLabel( statusBar() );
    statusBar()->addPermanentWidget( _fileNameLabel, 0 );

    // Set application icon
    setWindowIcon( QPixmap( "editor.png" ) );

    // The editor widget:
    _edit = new QTextEdit( this );
    _edit->setFocus();
    _edit->setWordWrapMode( QTextOption::WordWrap );
    _edit->document()->setModified( false );
    setCentralWidget( _edit );

    connect( _edit->document(), SIGNAL( contentsChanged() ),
             this, SLOT( slotTextChanged() ) );
    slotTextChanged();
}

void Editor::closeEvent( QCloseEvent* event )
{
    event->setAccepted( false ); // In case we cancel the closing, reject the event.
    slotQuit();
}

void Editor::slotFileOpen()
{
    _fileName = QFileDialog::getOpenFileName( this );
    QFile f( _fileName );
    if( !f.open( QIODevice::ReadOnly ) )
        return;

    _fileNameLabel->setText( _fileName.mid( _fileName.lastIndexOf( QDir::separator())+1));

    QTextStream stream( &f );
    _edit->setPlainText( stream.readAll() );
    _edit->document()->setModified( false );
    slotTextChanged();
}

void Editor::slotFileSave()
{
    QString fileName = QFileDialog::getSaveFileName( this, QString(), _fileName );
    if( !fileName.isEmpty() ) {
        QFile f( fileName );
        if( !f.open( QIODevice::WriteOnly ) ) return;
        _fileName = fileName;
        QTextStream t(&f);
        t << _edit->toPlainText();
        f.close();
        _edit->document()->setModified( false );
        _fileNameLabel->setText( _fileName.mid( _fileName.lastIndexOf( QDir::separator()) + 1));
        slotTextChanged();
    } else {
        // Cancel, do nothing
    }
}

void Editor::slotFilePrint()
{
    QPrinter printer;
    QPrintDialog dialog(&printer);
    if( dialog.exec() ) {
        _edit->document()->print( &printer );
        statusBar()->showMessage( tr("Printing completed"), 2000 );
    } else
        statusBar()->showMessage( tr("Printing aborted"), 2000 );
}

void Editor::slotQuit()
{
    if( _edit->document()->isModified() ) {
        int r = QMessageBox::warning( this, "Save before quit",
                                      "The document is not saved",
                                      "Save", "Discard", "Cancel" );
        switch( r ) {
        case 0: // Save
            slotFileSave();
            break;
        case 1: // Discard
            break;
        case 2: // cancel
            return;
        }
    }
    qApp->quit();
}

void Editor::slotAbout()
{
    QMessageBox::about( this, "About Text Editor", "Super wiz-bang text editor" );
}

void Editor::slotAboutQt()
{
    QMessageBox::aboutQt( this );
}

void Editor::slotTextChanged()
{
    bool modified = _edit->document()->isModified();
    _fileSaveAction->setEnabled( modified );
    _filePrintAction->setEnabled( modified );
}
