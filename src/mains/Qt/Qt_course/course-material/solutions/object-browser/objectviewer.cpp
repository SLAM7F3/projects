#include <QtGui>
#include "objectviewer.h"

ObjectViewer::ObjectViewer( QWidget* parent )
    :QTreeView( parent )
{
    connect( this, SIGNAL( doubleClicked( const QModelIndex& ) ), this, SLOT( displayWidget( const QModelIndex& ) ) );
}

void ObjectViewer::displayWidget( const QModelIndex& index )
{
    QObject* o = static_cast<QObject*>( index.internalPointer() );
    if ( !o->isWidgetType() )
        return;

    QWidget* widget = static_cast<QWidget*>( o );

    for ( int i = 0; i < 16; ++i ) {
        QPalette pal = widget->palette();
        for ( int role = QPalette::Foreground; role < QPalette::LinkVisited; ++ role ) {
            for ( int which = 0; which < 3; ++ which ) {
                QColor col = pal.color( (QPalette::ColorGroup) which,
                                        (QPalette::ColorRole) role  );
                int r = (col.red() + 16)%256;
                int g = (col.green() + 16)%256;
                int b = (col.blue() + 16)%256;
                pal.setColor( (QPalette::ColorGroup) which,
                              (QPalette::ColorRole) role, QColor( r,g,b ) );
            }
        }
        widget->setPalette( pal );
        widget->repaint();
        qApp->processEvents( QEventLoop::ExcludeUserInputEvents );
#ifdef Q_WS_WIN
		_sleep( 100 );
#else
		usleep( 100000 );
#endif
    }
}
