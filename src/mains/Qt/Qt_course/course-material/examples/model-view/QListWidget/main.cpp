#include <QtGui>

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    // Simple ListMode without icons
    {
        QListWidget* w = new QListWidget;
        for ( int i = 1; i < 10; ++i )
            w->addItem( QString("Item %1").arg(i) );
        w->show();
    }

    // ListMode with Icons
    {
        QListWidget* w = new QListWidget;
        for ( int i = 1; i < 10; ++i ) {
            QListWidgetItem* item = new QListWidgetItem( QString("Item %1").arg(i), w );
            item->setIcon( QPixmap( QString("%1.png").arg(i)));
        }
        w->show();
    }

    // IconMode
    {
        QListWidget* w = new QListWidget;
        for ( int i = 1; i < 10; ++i ) {
            QListWidgetItem* item = new QListWidgetItem( QString("Item %1").arg(i), w );
            item->setIcon( QPixmap( QString("%1.png").arg(i)));
        }
        w->setViewMode( QListView::IconMode );
        w->show();
    }

    return app.exec();
}
