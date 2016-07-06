#ifndef PERSON_H
#define PERSON_H
#include <q3canvas.h>

class Person :public Q3CanvasSprite 
{
public:
  Person( int z, const QString& filepattern, Q3Canvas* parent );
  void advance( int phase );
  int rtti() const { return 1000; }
private:
  QString _name;
  double dx, dy;
};

#endif /* PERSON_H */

