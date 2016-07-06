#include "Texture2BufferObjectKernel.h"

#include <cassert>
#include <GL/GLFramebufferObject.h>
#include <GL/GLShared.h>
#include <GL/GLTextureRectangle.h>
#include <GL/GLVertexBufferObject.h>
#include <GL/GLUtilities.h>
#include <time/StopWatch.h>

// static
Texture2BufferObjectKernel* Texture2BufferObjectKernel::create( QString args )
{
	Texture2BufferObjectKernel* pInstance = new Texture2BufferObjectKernel( args );
	pInstance->initializeGL();
	return pInstance;
}

// virtual
Texture2BufferObjectKernel::~Texture2BufferObjectKernel()
{	
	deleteOutputBufferObject();
}

// virtual
bool Texture2BufferObjectKernel::isInputComplete()
{
	return true;
}

// virtual
void Texture2BufferObjectKernel::makeDirty( QString inputPortName )
{
	if( inputPortName == "inputTexture" )
	{
		m_bReallocationNeeded = true;
	}
	m_pOutputPort->makeDirty();
}

// virtual
void Texture2BufferObjectKernel::compute( QString outputPortName )
{	
	if( m_bReallocationNeeded )
	{
		reallocate();		
	}

	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pInputTexture );

	m_pOutputBufferObject->bind( GLBufferObject::TARGET_PIXEL_PACK_BUFFER );
	if( m_bIsUnsignedByte )
	{
		glReadPixels( 0, 0, m_pInputTexture->getWidth(), m_pInputTexture->getHeight(),
			GL_RGB, GL_UNSIGNED_BYTE, NULL );
	}
	else
	{
		glReadPixels( 0, 0, m_pInputTexture->getWidth(), m_pInputTexture->getHeight(),
			GL_RGBA, GL_FLOAT, NULL );
	}
	m_pOutputBufferObject->unbind( GLBufferObject::TARGET_PIXEL_PACK_BUFFER );
}

//////////////////////////////////////////////////////////////////////////
// protected
//////////////////////////////////////////////////////////////////////////

// virtual
void Texture2BufferObjectKernel::initializeGL()
{
	GPUKernel::initializeGL();
}

// virtual					
void Texture2BufferObjectKernel::initializePorts()
{
	m_pInputTextureInputPort = addInputPort( "inputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );

	m_pOutputPort = addOutputPort( "outputBufferObject", KERNEL_PORT_DATA_TYPE_GL_BUFFER_OBJECT );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

Texture2BufferObjectKernel::Texture2BufferObjectKernel( QString args ) :

	GPUKernel( "Texture2BufferObject" ),

	m_pFBO( GLShared::getInstance()->getSharedFramebufferObject() ),

	m_bReallocationNeeded( true ),

	m_pInputTexture( NULL ), // invalid
	m_iWidth( -1 ), // invalid
	m_iHeight( -1 ), // invalid

	// output
	m_pOutputBufferObject( NULL )
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
		fprintf( stderr, "Texture2BufferObjectKernel: args must be \"ubyte\" or \"float\"\n" );
		exit( -1 ); // exit, not assert
	}
}

void Texture2BufferObjectKernel::deleteOutputBufferObject()
{
	if( m_pOutputBufferObject != NULL )
	{
		delete m_pOutputBufferObject;
		m_pOutputBufferObject = NULL;
	}
}

void Texture2BufferObjectKernel::reallocate()
{
	m_pInputTexture = m_pInputTextureInputPort->pullData().getGLTextureRectangleData();
	int inputTextureWidth = m_pInputTexture->getWidth();
	int inputTextureHeight = m_pInputTexture->getHeight();

	bool sizeChanged = false;
	if( m_iWidth != inputTextureWidth )
	{
		m_iWidth = inputTextureWidth;
		sizeChanged = true;
	}
	if( m_iHeight != inputTextureHeight )
	{
		m_iHeight = inputTextureHeight;
		sizeChanged = true;
	}

	if( sizeChanged )
	{
		deleteOutputBufferObject();

		if( m_bIsUnsignedByte )
		{
			m_pOutputBufferObject = new GLBufferObject( GLBufferObject::TARGET_PIXEL_PACK_BUFFER,
				GLBufferObject::USAGE_STATIC_COPY,
				m_iWidth * m_iHeight,
				3 * sizeof( ubyte ) );
		}
		else
		{
			m_pOutputBufferObject = new GLBufferObject( GLBufferObject::TARGET_PIXEL_PACK_BUFFER,
				GLBufferObject::USAGE_STATIC_COPY,
				m_iWidth * m_iHeight,
				4 * sizeof( float ) );
		}
		m_pOutputBufferObject->unbind( GLBufferObject::TARGET_PIXEL_PACK_BUFFER );

		m_pOutputPort->pushData( KernelPortData( m_pOutputBufferObject ) );
	}

	m_bReallocationNeeded = false;
}
