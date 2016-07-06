#include "BufferObject2TextureKernel.h"

#include <cassert>
#include <common/ArrayWithLength.h>
#include <GL/GLTextureRectangle.h>
#include <GL/GLVertexBufferObject.h>
#include <GL/GLUtilities.h>

// static
BufferObject2TextureKernel* BufferObject2TextureKernel::create( QString args )
{
	BufferObject2TextureKernel* pInstance = new BufferObject2TextureKernel( args );
	pInstance->initializeGL();
	return pInstance;
}

// virtual
BufferObject2TextureKernel::~BufferObject2TextureKernel()
{	
	deleteOutputTexture();
}

// virtual
bool BufferObject2TextureKernel::isInputComplete()
{
	return
	(
		m_pSizePort->isConnected() &&		
		m_pBufferObjectInputPort->isConnected()
	);
}

// virtual
void BufferObject2TextureKernel::makeDirty( QString inputPortName )
{
	if( inputPortName == "size" )
	{
		m_bReallocationNeeded = true;
	}
	m_pOutputPort->makeDirty();
}

// virtual
void BufferObject2TextureKernel::compute( QString outputPortName )
{	
	if( m_bReallocationNeeded )
	{
		reallocate();
		m_bReallocationNeeded = false;
	}

	GLBufferObject* pInputVBO = m_pBufferObjectInputPort->pullData().getGLBufferObjectData();
	
	pInputVBO->bind( GLBufferObject::TARGET_PIXEL_UNPACK_BUFFER );	

	if( m_bIsUnsignedByte )
	{
		m_pOutputTexture->setUnsignedByte3Data( NULL );
	}
	else
	{
		m_pOutputTexture->setFloat4Data( NULL );
	}
	pInputVBO->unbind( GLBufferObject::TARGET_PIXEL_UNPACK_BUFFER );
}

//////////////////////////////////////////////////////////////////////////
// protected
//////////////////////////////////////////////////////////////////////////

// virtual
void BufferObject2TextureKernel::initializeGL()
{
	GPUKernel::initializeGL();
}

// virtual					
void BufferObject2TextureKernel::initializePorts()
{
	m_pSizePort = addInputPort( "size", KERNEL_PORT_DATA_TYPE_SIZE_2D );
	m_pBufferObjectInputPort = addInputPort( "inputBufferObject", KERNEL_PORT_DATA_TYPE_GL_BUFFER_OBJECT );

	m_pOutputPort = addOutputPort( "outputTexture",	KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

BufferObject2TextureKernel::BufferObject2TextureKernel( QString args ) :

	GPUKernel( "BufferObject2Texture" ),

	m_bReallocationNeeded( true ),

	// qSize is automatically invalid

	// outputs
	m_pOutputTexture( NULL )
{
	if( args == "ubyte" )
	{
		m_bIsUnsignedByte = true;
	}
	else if( args == "float" )
	{
		m_bIsUnsignedByte = false;
	}
	else
	{
		fprintf( stderr, "BufferObject2Texture: args must be \"ubyte\" or \"float\"\n" );
		exit( -1 );
	}
}

void BufferObject2TextureKernel::deleteOutputTexture()
{
	if( m_pOutputTexture != NULL )
	{
		delete m_pOutputTexture;
		m_pOutputTexture = NULL;
	}
}

void BufferObject2TextureKernel::reallocate()
{
	QSize size = m_pSizePort->pullData().getSize2DData();
	if( size != m_qSize )
	{
		m_qSize = size;

		deleteOutputTexture();

		if( m_bIsUnsignedByte )
		{
			m_pOutputTexture = GLTextureRectangle::createUnsignedByte3Texture( size.width(), size.height() );
		}
		else
		{
			m_pOutputTexture = GLTextureRectangle::createFloat4Texture( size.width(), size.height(), 32 );
		}

		m_pOutputPort->pushData( KernelPortData( m_pOutputTexture ) );		
	}
}
