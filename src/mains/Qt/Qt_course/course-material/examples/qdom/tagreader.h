#ifndef TAGREADER_H
#define TAGREADER_H

#include <QDomElement>

class TagReader
{
public:
  void parse( const char* file );
protected:
  void parseNode( QDomElement elm, int offset );
};

#endif /* TAGREADER_H */

