#include "person.h"

Person::Person( int z, const QString& filepattern, Q3Canvas* parent ) :Q3CanvasSprite( 0, parent ), dx(0), dy(0)
{
  setSequence( new Q3CanvasPixmapArray( filepattern ) );

  int x = static_cast<int>( (canvas()->size().width() - width()) * (double) rand()/RAND_MAX );
  int y = static_cast<int>( (canvas()->size().height() -height() ) * (double) rand()/RAND_MAX );
  int vx = static_cast<int>( 10 * (double) rand()/RAND_MAX -5 );
  int vy = static_cast<int>( 10 * (double) rand()/RAND_MAX -5 );

  setX( x );
  setY( y );
  setZ( z );
  setXVelocity( vx );
  setYVelocity( vy );
}

void Person::advance( int phase )
{
  if ( phase == 0 ) {
    Q3CanvasItemList list = collisions( true );
    bool xCollision = false;
    bool yCollision = false;
    double xv = xVelocity();
    double yv = yVelocity();

    for ( Q3CanvasItemList::Iterator it = list.begin(); it != list.end(); ++it ){
      if ( (*it)->rtti() == 1000 ) {
        Person* other = dynamic_cast<Person*>(*it);

        // To decide whether this is a vertical or horizontal collision we need to know the horizonal
        // speed difference, as they can be this wide apart, and still colide in the next step.
        double xspeeddiff = QABS(xVelocity()) + QABS(other->xVelocity());

        if ( ( QABS(x()+width() - other->x() ) <= xspeeddiff ) ||
             ( QABS(x() - (other->x()+ other->width())) <= xspeeddiff ) ) {
          // horizontal collision
          dx += other->xVelocity();
          xCollision = true;
        }
        else {
          // vertical collision
          yCollision = true;
          dy += other->yVelocity();
        }
      }
      else {
        // Must be the text
        Q3CanvasItem* other = *it;
        if  (other->yVelocity() < 0 ) { // only lift items
          dy = - yVelocity();
          dy += other->yVelocity();
        }
      }
    }
    if ( xCollision )
      dx -= xv;
    if ( yCollision )
      dy -= yv;
  }
  else {
    setXVelocity( dx + xVelocity() );
    setYVelocity( dy + yVelocity() );
    dx = 0;
    dy = 0;

    if ( x() < 0  || x()+width()+xVelocity() > canvas()->width() ) {
      setXVelocity( - xVelocity() );
    }
    if ( y() < 0  || y()+height()+yVelocity() > canvas()->height() ) {
      setYVelocity( - yVelocity() );
    }
  }
  Q3CanvasSprite::advance( phase );
}

