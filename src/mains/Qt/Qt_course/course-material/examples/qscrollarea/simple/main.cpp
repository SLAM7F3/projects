#include <QtGui>

int main( int argc, char** argv )
{
  QApplication app( argc, argv );

  QScrollArea* sa = new QScrollArea( 0 );
  QWidget* w = new QWidget();
  w->setMinimumSize( 100, 20*2000 );
  w->resize(100,20*2000);
  sa->setWidget(w);
  sa->setWidgetResizable(true);

  for(int i = 0; i < 2000; ++i) {
    QCheckBox* b = new QCheckBox(QString("box #%1").arg(i), w );
    b->move(0, i*20);
  }

  sa->show();

  return app.exec();
}
