#include <QtGui>
#include "editor.h"
#include "ui_tableproperties.h"
#include "ui_searchreplace.h"
#include "replaceprogressdialog.h"
#include "floatingframedialog.h"

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

    // Insert Table
    QAction* insertTable = new QAction( this );
    insertTable->setText( "Insert Table" );
    connect( insertTable, SIGNAL( triggered() ), this, SLOT( slotInsertTable() ) );

    // Insert List
    QAction* insertList = new QAction( this );
    insertList->setText( "Insert List" );
    connect( insertList, SIGNAL( triggered() ), this, SLOT( slotInsertList() ) );

    // Insert Text Box
    QAction* insertTextBox = new QAction( this );
    insertTextBox->setText( "Insert Text Box" );
    connect( insertTextBox, SIGNAL( triggered() ), this, SLOT( slotInsertTextBox() ) );

    // Bold
    _boldface = new QAction( this );
    _boldface->setText( "Boldface" );
    _boldface->setIcon( QPixmap( "textbold.png" ) );
    _boldface->setCheckable( true );
    connect( _boldface, SIGNAL( checked( bool ) ), this, SLOT( slotBoldface( bool ) ) );

    // Italic
    _italic = new QAction( this );
    _italic->setText( "Italic" );
    _italic->setIcon( QPixmap( "textitalic.png" ) );
    _italic->setCheckable( true );
    connect( _italic, SIGNAL( checked( bool ) ), this, SLOT( slotItalic( bool ) ) );

    // Underline
    _underline = new QAction( this );
    _underline->setText( "Underline" );
    _underline->setIcon( QPixmap( "textunder.png" ) );
    _underline->setCheckable( true );
    connect( _underline, SIGNAL( checked( bool ) ), this, SLOT( slotUnderline( bool ) ) );

    // Search/Replace
    QAction* searchReplace = new QAction( this );
    searchReplace->setText( "Search and Replace" );
    connect( searchReplace, SIGNAL( triggered() ), this, SLOT( slotSearchReplace() ) );

    // -------------------------------------------------- Menubar
    QMenu* fileMenu = new QMenu( "&File", menuBar() );
    menuBar()->addMenu( fileMenu );

    fileMenu->addAction(fileOpenAction);
    fileMenu->addAction(_fileSaveAction);
    fileMenu->addAction(_filePrintAction);
    fileMenu->addAction( searchReplace );
    fileMenu->addSeparator();
    fileMenu->addAction(quitAction);

    QMenu* insert = new QMenu( "&Insert", menuBar() );
    menuBar()->addMenu( insert );

    insert->addAction( insertTable );
    insert->addAction( insertList );
    insert->addAction( insertTextBox );

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

    QToolBar* editBar = new QToolBar( "Edit", this );
    addToolBar( editBar );

    editBar->addAction( _boldface );
    editBar->addAction( _italic );
    editBar->addAction( _underline );

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
    connect( _edit, SIGNAL( currentCharFormatChanged( const QTextCharFormat& ) ),
             this, SLOT( updateButtonStates(  const QTextCharFormat& ) ) );
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

    _edit->setPlainText( f.readAll() );
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

void Editor::slotInsertTable()
{
    QDialog dialog( this );
    Ui::TableProperties props;
    props.setupUi( &dialog );
    if ( dialog.exec() ) {
        QTextTableFormat format;
        format.setBorder( 1 );
        format.setWidth( props.width->value() );

        QTextCursor cursor( _edit->textCursor());
        cursor.insertTable( props.rows->value(), props.columns->value(), format );
    }
}

void Editor::slotInsertList()
{
    QTextCursor cursor( _edit->textCursor());
    QTextBlockFormat block = cursor.blockFormat();
    QTextListFormat format;
    format.setStyle( QTextListFormat::ListDisc );
    format.setIndent( block.indent()+1 );
    cursor.insertList( format );
}

void Editor::slotBoldface( bool b )
{
    QTextCharFormat fmt;
    fmt.setFontWeight( b ? QFont::Bold : QFont::Normal );
    setFormat( fmt );
}

void Editor::slotItalic( bool italic )
{
    QTextCharFormat fmt;
    fmt.setFontItalic( italic );
    setFormat( fmt );
}

void Editor::slotUnderline( bool underline)
{
   QTextCharFormat fmt;
    fmt.setFontUnderline( underline );
    setFormat( fmt );
}

void Editor::setFormat( const QTextCharFormat& fmt )
{
    QTextCursor cursor( _edit->textCursor());
    cursor.mergeCharFormat(fmt);
    _edit->setTextCursor( cursor );
}



void Editor::slotSearchReplace()
{
    QDialog dialog( this );
    Ui::SearchReplace opts;
    opts.setupUi( &dialog );
    if ( !dialog.exec() )
        return;

    ReplaceProgressDialog replaceDialog( _edit, opts.search->text(), opts.replace->text(), this );
    replaceDialog.exec();
}

void Editor::slotInsertTextBox()
{
    FloatingFrameDialog dialog( this );
    if ( dialog.exec() ) {

        QTextCursor cursor( _edit->textCursor());
        QTextFrameFormat format;
        format.setWidth( dialog.length() );
        format.setPosition( dialog.position() );
        format.setBackground( Qt::yellow );
        cursor.insertFrame( format );
    }
}

void Editor::updateButtonStates( const QTextCharFormat& format )
{
    _italic->setChecked( format.fontItalic() );
    _underline->setChecked( format.fontUnderline() );
    _boldface->setChecked( format.fontWeight() == QFont::Bold );
}

