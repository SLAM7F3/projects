#include "kdtext.h"

KDText::KDText( Q3Canvas* parent ) :Q3CanvasText( "Klaralvdalens Datakonsult AB", parent )
{
  setX( 100 );
  setY( 500 );
  setZ( 0 );
  QFont f = font();
  f.setPixelSize(48);
  setFont(f);

  setXVelocity( 0 );
  setYVelocity( 0.1 );
}

void KDText::advance( int phase )
{
  double dv = 0.995;
  double m = 2;
  if ( phase == 0 ) {
    setYVelocity( yVelocity()*dv + m);

    // y() - current top of text
    // 48  - height of text
    // yVelocity() - This is the next step, which QCanvasText::advance will call.
    if ( y() +48 + yVelocity() > canvas()->height() ) {
      if ( QABS( yVelocity() ) < 1 ) {
        // Start over again.
        setYVelocity( 50 );
      }
      else
        setYVelocity( -yVelocity() );
      // This is to avoid that the text goes below the border.
      setY( canvas()->height()-48-yVelocity());
    }
  }

  Q3CanvasText::advance( phase );
}
