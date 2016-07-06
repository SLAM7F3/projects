#include <QtGui>

class Test : public QWidget {
Q_OBJECT

public:
    Test( QWidget* parent = 0 )
        :QWidget( parent ), _count(0)
    {
        QTimer* timer = new QTimer( this );
        connect( timer, SIGNAL( timeout() ), this, SLOT( next() ) );
        timer->start( 2000 );
        _pixmap = QPixmap("background.png");
        setFixedSize( _pixmap.size() );
    }

protected slots:
    void next()
    {
        _count = (_count + 1) % 10;
        repaint();
    }

protected:
    void paintEvent( QPaintEvent* )
    {
        // draw backGround;
        QPainter painter( this );
        painter.drawPixmap( 0, 0, _pixmap );

        // Draw Text
        QColor color = Qt::yellow;
        color.setAlpha( 256*_count/10 );
        painter.setPen( color );
        QFont f = painter.font();
        f.setPixelSize( 130 );
        painter.setFont( f );

        painter.drawText( 10, 150, "Hello World" );

        // Draw Rectangles
        color = Qt::yellow;
        for ( int i = 0; i < 10; ++i ) {
            color.setAlpha( 256*i/10 );
            painter.setPen( i == _count ? Qt::black : color );
            painter.setBrush( color );
            painter.drawRect( (i%5)*110 +150, 300 + (110 * (i/5)), 100, 100 );
        }
    }

private:
    int _count;
    QPixmap _pixmap;
};

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    Test* test = new Test;
    test->show();
    return app.exec();
}

#include "main.moc"
