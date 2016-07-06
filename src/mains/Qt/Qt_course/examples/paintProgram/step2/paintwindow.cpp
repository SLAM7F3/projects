#include "paintwindow.h"

#include "scribbleArea.h"

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
    _scribbleArea = new ScribbleArea( this );
    setCentralWidget( _scribbleArea );

    setupFileMenu();
    setupColorMenu();
    setupHelpMenu();

    statusBar()->showMessage("Ready!", 3000);
}

void PaintWindow::setupFileMenu()
{
    // Actions
    QAction* quit = new QAction( QPixmap( "icons/quit.png" ), "&Quit", this );
    quit->setShortcut( Qt::CTRL + Qt::Key_Q );
    quit->setStatusTip("This will quit the application unconditionally");
    quit->setWhatsThis("This is a button, stupid!");
    connect( quit, SIGNAL( triggered() ), qApp, SLOT( quit() ) );

    // Menubar
    QMenu* file = new QMenu( "&File", menuBar() );
    menuBar()->addMenu( file );
    file->addAction( quit );

    // Toolbar
    QToolBar* mainToolBar = new QToolBar( this );
    addToolBar( mainToolBar );
    mainToolBar->addAction( quit );
}


void PaintWindow::setupColorMenu()
{
    // Actions
    QActionGroup* actions = new QActionGroup( this );
    QAction* black  = new QAction( QPixmap( "icons/black.png" ),  "black",  actions );
    black->setData(Qt::black);

    QAction* red    = new QAction( QPixmap( "icons/red.png" ),    "red",    actions );
    red->setData(Qt::red);

    QAction* blue   = new QAction( QPixmap( "icons/blue.png" ),   "blue",   actions );
    blue->setData(Qt::blue);

    QAction* green  = new QAction( QPixmap( "icons/green.png" ),  "green",  actions );
    green->setData(Qt::green);

    QAction* yellow = new QAction( QPixmap( "icons/yellow.png" ), "yellow", actions );
    yellow->setData(Qt::yellow);

    actions->setExclusive( true );

    black ->setCheckable( true );
    red   ->setCheckable( true );
    blue  ->setCheckable( true );
    green ->setCheckable( true );
    yellow->setCheckable( true );

    connect( actions, SIGNAL( triggered( QAction* ) ),
             this, SLOT( slotChangeColor( QAction* ) ) );
    connect( this, SIGNAL( colorChange( const QColor & ) ),
             _scribbleArea, SLOT( slotChangeColor( const QColor& ) ) );

    black->setChecked( true );

    // Menubar
    QMenu* colorMenu = new QMenu( "&Colors", menuBar() );
    menuBar()->addMenu( colorMenu );
    colorMenu->addActions( actions->actions() );

    // Toolbar
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

    // Menubar
    QMenu* helpMenu =  new QMenu("&Help", menuBar() );
    menuBar()->addMenu( helpMenu );
    helpMenu->addAction(about);
    helpMenu->addAction(aboutQt);
}

void PaintWindow::slotChangeColor( QAction* action )
{
    QColor color = qvariant_cast<QColor>(action->data());

    emit colorChange( color );
    // Similar: _scribbleArea->slotChangeColor( color )
}

void PaintWindow::slotAbout()
{
    QMessageBox::information( this, "About The PaintWindow Program",
                              "This is the paint program\n"
                              "(C) 2000-2006 by Matthias Kalle Dalheimer\n"
                              "and Jesper K. Pedersen" );
}

void PaintWindow::slotAboutQt()
{
    QMessageBox::aboutQt( this, "About Qt" );
}
