#include "paintwindow.h"
#include <QFileDialog>
#include <QScrollArea>
#include <QMessageBox>
#include <QActionGroup>
#include <QToolBar>
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include "configDialog.h"

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
    setupShapesMenu();
    setupHelpMenu();

    connect( this, SIGNAL( load( const QString& ) ),_scribbleArea, SLOT( slotLoad( const QString& ) ) );
    connect( this, SIGNAL( save( const QString& ) ),_scribbleArea, SLOT( slotSave( const QString& ) ) );
    connect( this, SIGNAL( print() ),_scribbleArea, SLOT( slotPrint() ) );

    _configDialog = 0;
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
    connect ( print, SIGNAL( triggered() ), this, SIGNAL( print() ) );

    // Configure
    QAction* configure = new QAction( "&Configure", this );
    connect ( configure, SIGNAL( triggered() ), this, SLOT( slotConfigure() ) );

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
    file->addSeparator();
    file->addAction(configure);
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

    QToolBar* colorBar = new QToolBar( this );
    addToolBar( colorBar );

    QMenu* colorMenu = new QMenu( "&Colors", menuBar() );
    menuBar()->addMenu( colorMenu );

    colorBar->addActions( actions->actions() );
    colorMenu->addActions( actions->actions() );

    connect( actions, SIGNAL( triggered( QAction* ) ),
             this, SLOT( slotChangeColor( QAction* ) ) );
    connect( this, SIGNAL( colorChange( const QColor & ) ),
             _scribbleArea, SLOT( slotChangeColor( const QColor& ) ) );

    _red->setChecked( true );
}

void PaintWindow::setupHelpMenu()
{
    QAction* about = new QAction( "&About This Program", this );
    connect( about, SIGNAL( triggered() ), this, SLOT( slotAbout() ) );

    QAction* aboutQt = new QAction( "About &Qt", this );
    connect( aboutQt, SIGNAL( triggered() ), this, SLOT( slotAboutQt() ) );

    QMenu* helpMenu =  new QMenu("&Help", menuBar() );
    menuBar()->addSeparator();
    menuBar()->addMenu( helpMenu );

    helpMenu->addAction(about);
    helpMenu->addAction(aboutQt);
}

void PaintWindow::setupShapesMenu(  )
{
    // Actions
    QActionGroup* grp = new QActionGroup( this );
    _line = new QAction( QPixmap("icons/line.png"), "line", grp );
    _rect = new QAction( QPixmap("icons/rectangle.png"), "rectangle", grp );
    _ellipse = new QAction( QPixmap("icons/ellipse.png"), "ellipse", grp );
    _image = new QAction( QPixmap("icons/image.png"), "image", grp );
    _line->setCheckable( true );
    _rect->setCheckable( true );
    _ellipse->setCheckable( true );
    _image->setCheckable( true );

    connect( grp, SIGNAL( triggered( QAction* ) ), this, SLOT( slotShapeMenu( QAction* ) ) );
    connect( this, SIGNAL( shapeChange( int ) ),
             _scribbleArea, SLOT( setShape( int ) ) );
    connect( this, SIGNAL( drawPixmap( const QPixmap& ) ),
             _scribbleArea, SLOT( setPixmap( const QPixmap& ) ) );
    _line->setChecked( true );

    // Menubar
    QMenu* shapeMenu = new QMenu( "&Shape", this );
    menuBar()->addMenu( shapeMenu );
    shapeMenu->addActions( grp->actions() );

    // Toolbar
    QToolBar* shapeToolBar = new QToolBar( this );
    addToolBar( shapeToolBar );
    shapeToolBar->addActions( grp->actions() );

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
    if( !filename.isEmpty() )
        emit load( filename );
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
    if( !filename.isEmpty() )
        emit save( filename );
}

void PaintWindow::slotShapeMenu( QAction* action )
{
    if ( action == _line )
        emit shapeChange( ScribbleArea::Line );
    else if ( action == _rect )
        emit shapeChange( ScribbleArea::Rectangle );
    else if ( action == _ellipse )
        emit shapeChange( ScribbleArea::Ellipse );
    else if ( action == _image ) {
        QString fileName = QFileDialog::getOpenFileName( this, QString(), QString(), "*.bmp *.png *.jpg" );
        if ( !fileName.isEmpty() ) {
            QPixmap pix( fileName );
            if ( !pix.isNull() ) {
                emit drawPixmap( pix );
            }
        }
    }
}

void PaintWindow::slotConfigure()
{
    if ( !_configDialog) {
        _configDialog = new ConfigDialog( this );
        connect( _configDialog, SIGNAL( setPen( int, PenStyle ) ),
                 _scribbleArea, SLOT( slotSetPen( int, PenStyle ) ) );
        connect( _configDialog, SIGNAL( setBrush( BrushStyle ) ),
                 _scribbleArea, SLOT( slotSetBrush( BrushStyle ) ) );
        connect( _configDialog, SIGNAL( setBrush( QPixmap ) ),
                 _scribbleArea, SLOT( slotSetBrush( QPixmap ) ) );
    }
    _configDialog->show();
}
