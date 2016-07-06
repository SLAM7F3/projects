#ifndef KERNEL_PORT_DATA_H
#define KERNEL_PORT_DATA_H

#include <QSize>
#include <QString>
#include <QVector>
#include <common/BasicTypes.h>
#include <common/ArrayWithLength.h>

#include "KernelPortDataType.h"

class GLBufferObject;
class GLTextureRectangle;

class KernelPortData
{
  public:

   static KernelPortData unserialize( QString value );

   // Do not use.  Used just for initializing.
   KernelPortData();
   KernelPortData( bool boolValue );
   KernelPortData( float floatValue );
   KernelPortData( int intValue );
   KernelPortData( QSize size2DValue );
   KernelPortData( QString stringValue );
   KernelPortData( QVector< float > floatArrayValue );
   KernelPortData( ArrayWithLength< ubyte > arrayWithLengthValue );
   KernelPortData( GLBufferObject* glBufferObjectValue );
   KernelPortData( GLTextureRectangle* glTextureRectangleValue );
   KernelPortData( QVector< GLTextureRectangle* > glTextureRectangleArrayValue );

   // TODO: copy constructor, assignment operator, destructor

   KernelPortDataType getType() const;

   bool getBoolData() const;
   float getFloatData() const;
   int getIntData() const;
   QSize getSize2DData() const;
   QString getFilenameData() const;
   QVector< float > getFloatArrayData() const;
   ArrayWithLength< ubyte > getUnsignedByteArrayData() const;
   GLBufferObject* getGLBufferObjectData() const;
   GLTextureRectangle* getGLTextureRectangleData() const;
   QVector< GLTextureRectangle* > getGLTextureRectangleArrayData() const;

  private:

   KernelPortDataType m_type;

   bool m_bBoolData;

   float m_fFloatData;

   int m_iIntData;

   QSize m_qSize2DData;

   QString m_qsStringData;

   QVector< float > m_qvFloatArrayData;
   ArrayWithLength< ubyte > m_aubUnsignedByteArrayData;

   GLBufferObject* m_pBufferObjectData;
   GLTextureRectangle* m_pTextureRectangleData;
   QVector< GLTextureRectangle* > m_qvTextureRectangleArrayData ;
};

#endif // KERNEL_PORT_DATA_H
