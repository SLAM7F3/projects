#include "KernelPortData.h"

#include <cassert>
#include <GL/GLBufferObject.h>
#include <GL/GLTextureRectangle.h>
#include <QStringList>

// static
KernelPortData KernelPortData::unserialize( QString value )
{
   QStringList tokens = value.split( " ", QString::SkipEmptyParts );
   if( tokens.size() < 2 )
   {
      printf( "Attempting to unserialize string \"%s\".  Number of tokens must be at least 2.",
              qPrintable( value ) );
      assert( false );
   }

   QString dataType = tokens[0];

   if( dataType == "bool" )
   {
      bool dataValue = false;
      if( tokens[1] == "true" )
      {
         dataValue = true;
      }		

      return KernelPortData( dataValue );
   }
   else if( dataType == "float" )
   {
      return KernelPortData( tokens[1].toFloat() );
   }
   else if( dataType == "int" )
   {
      return KernelPortData( tokens[1].toInt() );
   }
   else if( dataType == "floatArray" )
   {
      QVector< float > dataValue;
      for( int i = 1; i < tokens.size(); ++i )
      {
         dataValue.append( tokens[i].toFloat() );
      }

      return KernelPortData( dataValue );
   }
   else if( dataType == "string" )
   {
      return KernelPortData( tokens[1] );
   }

   return KernelPortData();
}

KernelPortData::KernelPortData() :

   m_type( KERNEL_PORT_DATA_TYPE_NULL )

{

}

KernelPortData::KernelPortData( bool boolValue ) :

   m_type( KERNEL_PORT_DATA_TYPE_BOOL )

{	
   m_bBoolData = boolValue;
}

KernelPortData::KernelPortData( float floatValue ) :

   m_type( KERNEL_PORT_DATA_TYPE_FLOAT )

{	
   m_fFloatData = floatValue;	
}

KernelPortData::KernelPortData( int intValue ) :

   m_type( KERNEL_PORT_DATA_TYPE_INT )

{
   m_iIntData = intValue;
}

KernelPortData::KernelPortData( QSize size2DValue ) :

   m_type( KERNEL_PORT_DATA_TYPE_SIZE_2D )

{
   m_qSize2DData = size2DValue;
}

KernelPortData::KernelPortData( QString stringValue ) :

   m_type( KERNEL_PORT_DATA_TYPE_STRING )

{
   m_qsStringData = stringValue;
}

// static
KernelPortData::KernelPortData( QVector< float > floatArrayValue ) :

   m_type( KERNEL_PORT_DATA_TYPE_FLOAT_ARRAY )

{
   m_qvFloatArrayData = floatArrayValue;	
}

// static
KernelPortData::KernelPortData( ArrayWithLength< ubyte > arrayWithLengthValue ) :

   m_type( KERNEL_PORT_DATA_TYPE_UNSIGNED_BYTE_ARRAY )

{	
   m_aubUnsignedByteArrayData = arrayWithLengthValue;
}

// static
KernelPortData::KernelPortData( GLBufferObject* glBufferObjectValue ) :

   m_type( KERNEL_PORT_DATA_TYPE_GL_BUFFER_OBJECT )

{	
   m_pBufferObjectData = glBufferObjectValue;
}

// static
KernelPortData::KernelPortData( GLTextureRectangle* glTextureRectangleValue ) :

   m_type( KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE )

{	
   m_pTextureRectangleData = glTextureRectangleValue;
}

// static
KernelPortData::KernelPortData( QVector< GLTextureRectangle* > glTextureRectangleArrayValue ) :

   m_type( KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE_ARRAY )

{	
   m_qvTextureRectangleArrayData = glTextureRectangleArrayValue;	
}

KernelPortDataType KernelPortData::getType() const
{
   return m_type;
}

bool KernelPortData::getBoolData() const
{
   return m_bBoolData;
}

float KernelPortData::getFloatData() const
{
   return m_fFloatData;
}

int KernelPortData::getIntData() const
{
   return m_iIntData;
}

QSize KernelPortData::getSize2DData() const
{
   return m_qSize2DData;
}

QString KernelPortData::getFilenameData() const
{
   return m_qsStringData;
}

QVector< float > KernelPortData::getFloatArrayData() const
{
   return m_qvFloatArrayData;
}

ArrayWithLength< ubyte > KernelPortData::getUnsignedByteArrayData() const
{
   return m_aubUnsignedByteArrayData;
}

GLBufferObject* KernelPortData::getGLBufferObjectData() const
{
   return m_pBufferObjectData;
}

GLTextureRectangle* KernelPortData::getGLTextureRectangleData() const
{
   return m_pTextureRectangleData;
}

QVector< GLTextureRectangle* > KernelPortData::getGLTextureRectangleArrayData() const
{
   return m_qvTextureRectangleArrayData;
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////
