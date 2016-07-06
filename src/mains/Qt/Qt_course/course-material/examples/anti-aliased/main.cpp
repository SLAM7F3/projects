#include <QtGui>

class Test : public QWidget {

public:
    Test( QWidget* parent = 0 ) :QWidget( parent ) {}

protected:
    virtual void paintEvent( QPaintEvent* )
    {
        QPainter painter( this );

        QPen pen;
        pen.setWidth( 3 );
        painter.setPen( pen );

        // One line without anti-aliasing
        painter.drawLine(0, 10, width()-10, height());

        // One line with anti-aliasing
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawLine(10, 0, width(), height()-10);
    }
};


int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    Test* test = new Test;
    test->show();

    return app.exec();
}
