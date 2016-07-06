#include "paintwindow.h"

#include "scribbleArea.h"

#include <QFileDialog>
#include <QScrollArea>
#include <QMessageBox>
#include <QActionGroup>
#include <QToolBar>
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>

PaintWindow::PaintWindow( QWidget* parent )
    : QMainWindow( parent )
{
    QScrollArea* view = new QScrollArea( this );
    setCentralWidget( view );
    _scribbleArea = new ScribbleArea( view );
    _scribbleArea->resize(1000,1000);
    view->setWidget( _scribbleArea );

    setupFileMenu();
    setupColorMenu();
    setupHelpMenu();

    statusBar()->showMessage("Ready!", 3000);
}

void PaintWindow::setupFileMenu()
{
    // -------------------------------------------------- Actions

    // Load
    QAction* load = new QAction( QPixmap( "icons/load.png" ), "&Load", this );
    load->setShortcut( Qt::CTRL + Qt::Key_L );
    connect ( load, SIGNAL( triggered() ), this, SLOT( slotLoad() ) );

    // Save
    QAction* save = new QAction(  QPixmap( "icons/save.png" ), "&Save", this );
    save->setShortcut( Qt::CTRL + Qt::Key_S );
    connect ( save, SIGNAL( triggered() ), this, SLOT( slotSave() ) );

    // Print
    QAction* print = new QAction(  QPixmap( "icons/print.png" ), "&Print", this );
    print->setShortcut( Qt::CTRL + Qt::Key_P );
    connect ( print, SIGNAL( triggered() ), _scribbleArea, SLOT( slotPrint() ) );

    // Quit
    QAction* quit = new QAction( QPixmap( "icons/quit.png" ), "&Quit", this );
    quit->setShortcut( Qt::CTRL + Qt::Key_Q );
    quit->setStatusTip("This will quit the application unconditionally");
    quit->setWhatsThis("This is a button, stupid!");
    connect( quit, SIGNAL( triggered() ), qApp, SLOT( quit() ) );

    // -------------------------------------------------- Menu bar
    QMenu* file = new QMenu( "&File", menuBar() );
    menuBar()->addMenu( file );
    file->addAction( load );
    file->addAction( save );
    file->addAction( print );
    file->addAction( quit );

    // -------------------------------------------------- Tool bar
    QToolBar* mainToolBar = new QToolBar( this );
    addToolBar( mainToolBar );
    mainToolBar->addAction( load );
    mainToolBar->addAction( save );
    mainToolBar->addAction( print );
    mainToolBar->addAction( quit );
}


void PaintWindow::setupColorMenu()
{
    // Actions
    QActionGroup* actions = new QActionGroup( this );
    _black  = new QAction( QPixmap( "icons/black.png" ),  "black",  actions );
    _red    = new QAction( QPixmap( "icons/red.png" ),    "red",    actions );
    _blue   = new QAction( QPixmap( "icons/blue.png" ),   "blue",   actions );
    _green  = new QAction( QPixmap( "icons/green.png" ),  "green",  actions );
    _yellow = new QAction( QPixmap( "icons/yellow.png" ), "yellow", actions );
    actions->setExclusive( true );

    _black ->setCheckable( true );
    _red   ->setCheckable( true );
    _blue  ->setCheckable( true );
    _green ->setCheckable( true );
    _yellow->setCheckable( true );

    connect( actions, SIGNAL( triggered( QAction* ) ),
             this, SLOT( slotChangeColor( QAction* ) ) );
    connect( this, SIGNAL( colorChange( const QColor & ) ),
             _scribbleArea, SLOT( slotChangeColor( const QColor& ) ) );

    _black->setChecked( true );

    // Menu bar
    QMenu* colorMenu = new QMenu( "&Colors", menuBar() );
    menuBar()->addMenu( colorMenu );
    colorMenu->addActions( actions->actions() );

    // Tool bar
    QToolBar* colorBar = new QToolBar( this );
    addToolBar( colorBar );
    colorBar->addActions( actions->actions() );
}

void PaintWindow::setupHelpMenu()
{
    // Actions
    QAction* about = new QAction( "&About This Program", this );
    connect( about, SIGNAL( triggered() ), this, SLOT( slotAbout() ) );

    QAction* aboutQt = new QAction( "About &Qt", this );
    connect( aboutQt, SIGNAL( triggered() ), this, SLOT( slotAboutQt() ) );

    // Menu bar
    QMenu* helpMenu =  new QMenu("&Help", menuBar() );
    menuBar()->addSeparator();
    menuBar()->addMenu( helpMenu );
    helpMenu->addAction(about);
    helpMenu->addAction(aboutQt);
}

void PaintWindow::slotChangeColor( QAction* action )
{
    QColor color;

    if ( action == _black )
        color = Qt::black;
    else if ( action == _red )
        color = Qt::red;
    else if ( action == _blue )
        color = Qt::blue;
    else if ( action == _green )
        color = Qt::green;
    else if ( action == _yellow )
        color = Qt::yellow;

    emit colorChange( color );
    // Similar: _scribleArea->slotChangeColor( color )
}

void PaintWindow::slotAbout()
{
    QMessageBox::information( this, "About The PaintWindow Program",
                              "This is the paint program\n"
                              "(C) 2000-2005 by Matthias Kalle Dalheimer\n"
                              "and Jesper K. Pedersen"
        );
}

void PaintWindow::slotAboutQt()
{
    QMessageBox::aboutQt( this, "About Qt" );
}

void PaintWindow::slotLoad()
{
    /* Open a file dialog for loading. The default directory is the
     * current directory, the filter *.bmp.
     */
    QString filename = QFileDialog::getOpenFileName( this, "Open Image", ".", "Images(*.bmp *.jpg *.png)" );
    if ( !filename.isEmpty() )
        _scribbleArea->slotLoad( filename ); // Note we call this slot as a normal method
}


/**
 * This is the save equivalent to slotLoad(). Again we just ask for a
 * filename and emit a signal.
 */
void PaintWindow::slotSave()
{
    /* Open a file dialog for saving. The default directory is the
     * current directory, the filter *.bmp.
     */
    QString filename = QFileDialog::getSaveFileName( this, QString(), QString(),"Images(*.bmp *.jpg *.png)" );
    if ( !filename.isEmpty() )
        _scribbleArea->slotSave( filename );  // Note we call this slot as a normal method
}
