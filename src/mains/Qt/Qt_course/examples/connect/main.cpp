#include <QtGui>

int main( int argc, char** argv ) {
  QApplication app( argc, argv );

  QSlider* slider = new QSlider( Qt::Horizontal );
  slider->setRange( 0, 100 );
  slider->setValue( 40 );

  QLCDNumber* num = new QLCDNumber;
  // Ensure that the widget can grow when resizing the window, ignore this for now
  num->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
  num->display( 40 );

  QObject::connect( slider, SIGNAL( valueChanged( int ) ), num, SLOT( display( int ) ) );


  QWidget* top = new QWidget;
  QHBoxLayout* layout = new QHBoxLayout( top );
  layout->addWidget( slider );
  layout->addWidget( num );

  top->show();
  return app.exec();
}
