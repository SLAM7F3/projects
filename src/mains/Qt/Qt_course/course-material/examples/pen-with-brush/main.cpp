#include <QtGui>

class Test : public QWidget {

public:
    Test( QWidget* parent = 0 )
        : QWidget( parent )
    {
        QFont fnt = font();
        fnt.setPixelSize( 100 );
        QFontMetrics fm(fnt);
        setFixedSize( fm.width( "Hello World" ), fm.height() );
    }

protected:

    void paintEvent( QPaintEvent* )
    {
        QPainter painter( this );
        QFont font = painter.font();
        font.setPixelSize( 100 );
        painter.setFont( font );
        QFontMetrics fm(font);

        QLinearGradient gradient( 0, 0, fm.width("Hello World"), fm.height());
        gradient.setColorAt(0, Qt::red);
        gradient.setColorAt(0.5, Qt::green);
        gradient.setColorAt(1, Qt::blue);
        QBrush b( gradient );
        QPen pen( b, 0 );
        pen.setStyle( Qt::SolidLine );
        painter.setPen( pen );
        painter.drawText( 0, 100, "Hello World" );
    }
};


int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    Test* test = new Test;
    test->show();

    return app.exec();
}
