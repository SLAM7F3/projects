#include <QtGui>

class Coord :public QWidget
{
protected:
  virtual void paintEvent(QPaintEvent *)
  {
    QPainter painter(this);
    painter.setWindow( -100,-100,200,200 );
    painter.drawLine( -100,0,100,0 );
    painter.drawLine( 0, -100, 0, 100 );

    QPen pen;
    pen.setWidth(3);
    pen.setColor( Qt::blue );
    painter.setPen( pen );
    painter.drawLine( 0,0,100,100 );

    QMatrix matrix( 1, 0, 0, -1, 0, 0 );
    painter.setMatrix( matrix );
    pen.setColor( Qt::red );
    pen.setStyle( Qt::DotLine );
    painter.setPen( pen );
    painter.drawLine( 0,0,100,100 );
  }
};


int main( int argc, char** argv )
{
  QApplication app(argc,argv);
  Coord* coord = new Coord();
  coord->resize(200,200);
  coord->show();

  return app.exec();
}
