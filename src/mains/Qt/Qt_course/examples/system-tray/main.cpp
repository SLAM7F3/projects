#include <QtGui>

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    if ( !QSystemTrayIcon::isSystemTrayAvailable() ) {
        QMessageBox::warning( 0, "No System Tray", "System Tray is not available" );
    }
    else {
        QSystemTrayIcon* trayIcon = new QSystemTrayIcon( QIcon( "black.png" ) );

        QMenu* menu = new QMenu;
        QAction* quit = new QAction( "Quit", 0 );
        menu->addAction( quit );
        QObject::connect( quit, SIGNAL( triggered() ), qApp, SLOT( quit() ) );
        trayIcon->setContextMenu( menu );

        trayIcon->showMessage( "Hello World", "Hello Qt world, this is a tray message" );
        trayIcon->show();
    }

    QObject::connect( qApp, SIGNAL( lastWindowClosed() ), qApp, SLOT( quit() ) );
    return app.exec();
}
