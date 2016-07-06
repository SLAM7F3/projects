#ifndef KDTEXT_H
#define KDTEXT_H

#include <q3canvas.h>

class KDText :public Q3CanvasText 
{
public:
  KDText( Q3Canvas* parent );
  void advance( int phase );
};
#endif /* KDTEXT_H */

