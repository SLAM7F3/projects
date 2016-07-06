#include <QtGui>
class Test :public QWidget
{
public:
    virtual void paintEvent( QPaintEvent * )
        {
            QPainter p(this);
            p.setBrush( Qt::red );
            p.drawRect( QRectF(2.0, 2.0, 10.0, 10.0) );
            p.fillRect( 2, 15, 10, 10, Qt::black );
            p.setRenderHint( QPainter::Antialiasing );
            p.drawRect( QRectF(2.0, 30.0, 10.0, 10.0) );
        }
};


int main( int argc, char** argv ) {
    QApplication app( argc, argv );
    Test* test = new Test;
    test->resize( 20, 50 );
    test->show();

    return app.exec();
}
