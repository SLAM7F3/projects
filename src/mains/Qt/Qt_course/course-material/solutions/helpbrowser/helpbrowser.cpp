#include <QtGui>
#include "helpbrowser.h"

HelpBrowser::HelpBrowser( QWidget* parent )
    : QDialog( parent )
{
    _browser = new QTextBrowser;
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->addWidget( _browser );

    qApp->installEventFilter( this );
    resize( 600,400 );
    hide();
}

void HelpBrowser::setSource( const QString& name )
{
    _browser->setSource( name );
}

void HelpBrowser::setHelpPath( const QString& path )
{
    _browser->setSearchPaths( QStringList() << path );
}

void HelpBrowser::setHelp(QWidget* widget, QString page)
{
    _helpMap.insert( widget, page );
}

// I can't use sender as this will be the widget with keyboard focus.
bool HelpBrowser::eventFilter( QObject* , QEvent* event )
{
    if ( event->type() == QEvent::KeyPress &&
         dynamic_cast<QKeyEvent*>(event)->key() == Qt::Key_F1 ) {

        QWidget* widget = QApplication::widgetAt( QCursor::pos() );

        if ( widget && _helpMap.contains( widget ) ) {
            QString page = _helpMap[widget];
            setSource( page );
            show();
        }
    }
    return false;
}

