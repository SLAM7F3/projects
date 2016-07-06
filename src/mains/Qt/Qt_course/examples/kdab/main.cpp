#include <QtGui>
#include "kdab.h"
#include "kdtext.h"
#include "person.h"

int main( int argc, char** argv )
{
  QApplication app(argc,argv);

  // Setup the canvas
  Q3Canvas* canvas = new Q3Canvas( 0 );
  canvas->resize( 900, 700 );
  canvas->setAdvancePeriod( 10 );
  canvas->setBackgroundColor( QColor(0xd4,0xff,0xc6) );

  // The persons.
  Person* kalle = new Person( 2, "picts/kalle.png", canvas );
  Person* jesper = new Person( 1, "picts/jesper.png", canvas );
  Person* steffen = new Person( 1, "picts/steffen.png", canvas );
  Person* lutz = new Person( 1, "picts/lutz.png", canvas );
  Person* tanja = new Person( 1, "picts/tanja.png", canvas );
  Person* david = new Person( 1, "picts/david.png", canvas );
  kalle->show();
  jesper->show();
  steffen->show();
  lutz->show();
  tanja->show();
  david->show();

  // The jumping text
  KDText* text = new KDText( canvas );
  text->show();

#if 1
  KDAB* kdab1 = new KDAB( canvas, 0 );
  kdab1->resize( 950, 750 );
  kdab1->show();
#endif
#if 1
  KDAB* kdab2 = new KDAB( canvas, 0 );
  QMatrix wm;
  wm.scale( 0.5, 0.5 );   // Zooms in by 2 times
  wm.rotate( 90 );    // Rotates 90 degrees clockwise
  wm.translate( 0, -1000);

  kdab2->setWorldMatrix( wm );
  kdab2->resize( 510, 510);
  kdab2->show();
#endif


  return app.exec();
}
