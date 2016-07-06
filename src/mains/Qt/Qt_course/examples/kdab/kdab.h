#ifndef KDAB_H
#define KDAB_H
#include <q3canvas.h>
#include <qpoint.h>
class Person;

class KDAB :public Q3CanvasView 
{
public:
  KDAB( Q3Canvas* canvas, QWidget* parent, const char* name = 0 );
};


#endif /* KDAB_H */

