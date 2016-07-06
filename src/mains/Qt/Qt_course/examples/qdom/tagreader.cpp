#include <QtGui>
#include "tagreader.h"

void TagReader::parse( const char* filename )
{
  QFile file( filename );
  if ( file.open( QIODevice::ReadOnly ) ) {
    QDomDocument doc;
    QString msg;
    bool ok = doc.setContent( &file, & msg );
    if ( !ok ) {
        qDebug() << "Error parsing " << filename << ": " << msg;
      return;
    }
    parseNode( doc.documentElement(), 0 );
  }
}

void TagReader::parseNode( QDomElement elm, int offset )
{
  QString name = QString("").rightJustified( offset ) + elm.tagName();
  qDebug() << name;
  for ( QDomNode child = elm.firstChild(); !child.isNull(); child = child.nextSibling() ) {
    if ( child.isElement() ) {
      parseNode( child.toElement(), offset+4 );
    }
  }
}

