#include "RGB2VBOKernel.h"

#include <cassert>
#include <GL/GLVertexBufferObject.h>
#include <GL/GLUtilities.h>

// static
RGB2VBOKernel* RGB2VBOKernel::create( QString args )
{
	RGB2VBOKernel* pInstance = new RGB2VBOKernel( args );
	pInstance->initializeGL();
	return pInstance;
}

// virtual
RGB2VBOKernel::~RGB2VBOKernel()
{
	cleanup();
}

// virtual
bool RGB2VBOKernel::isInputComplete()
{
	return m_pInputRGBInputPort->isConnected();
}

// virtual
void RGB2VBOKernel::makeDirty( QString inputPortName )
{
	// 1 in 1 out
	m_bReallocationNeeded = true;
	m_pBufferObjectOutputPort->makeDirty();	
}

// virtual
void RGB2VBOKernel::compute( QString outputPortName )
{
	if( m_bReallocationNeeded )
	{
		reallocate();
	}

	m_pOutputVBO->bind( GLBufferObject::TARGET_ARRAY_BUFFER );

	if( m_bIsUnsignedByte )
	{
		m_pOutputVBO->setUnsignedByteSubData
		(
			GLBufferObject::TARGET_ARRAY_BUFFER,
			m_aubInputRGB.data(),
			m_nBytes / 3
		);
	}
	else
	{
		m_pOutputVBO->setFloatSubData
		(
			GLBufferObject::TARGET_ARRAY_BUFFER,
			m_qvfInputRGB.constData(),
			m_qvfInputRGB.size() / 4
		);
	}

	m_pOutputVBO->unbind( GLBufferObject::TARGET_ARRAY_BUFFER );
}

//////////////////////////////////////////////////////////////////////////
// protected
//////////////////////////////////////////////////////////////////////////

// virtual
void RGB2VBOKernel::initializeGL()
{
	GPUKernel::initializeGL();
}

// virtual
void RGB2VBOKernel::initializePorts()
{
	// -- Input Ports --
	if( m_bIsUnsignedByte )
	{
		m_pInputRGBInputPort = addInputPort( "inputRGBArray", KERNEL_PORT_DATA_TYPE_UNSIGNED_BYTE_ARRAY );
	}
	else
	{
		m_pInputRGBInputPort = addInputPort( "inputRGBArray", KERNEL_PORT_DATA_TYPE_FLOAT_ARRAY );
	}

	// -- Output Ports --
	m_pBufferObjectOutputPort = addOutputPort( "outputBufferObject", KERNEL_PORT_DATA_TYPE_GL_BUFFER_OBJECT );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

RGB2VBOKernel::RGB2VBOKernel( QString args ) :

	GPUKernel( "RGB2VBO" ),

	m_bReallocationNeeded( true ),

	m_nBytes( -1 ), // invalid so it will be != on first comparison
	m_iWidth( -1 ), // invalid so it will be != on first comparison
	m_iHeight( -1 ), // invalid so it will be != on first comparison

	// outputs
	m_pOutputVBO( NULL )
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
		fprintf( stderr, "RGB2VBOKernel: args must be \"ubyte\" or \"float\"\n" );
		exit( -1 );
	}
}

void RGB2VBOKernel::cleanup()
{
	if( m_pOutputVBO != NULL )
	{
		delete m_pOutputVBO;
		m_pOutputVBO = NULL;
	}
}

void RGB2VBOKernel::reallocate()
{
	if( m_bIsUnsignedByte )
	{
		// there's only one input port
		m_aubInputRGB = m_pInputRGBInputPort->pullData().getUnsignedByteArrayData();
		int nBytes = m_aubInputRGB.length();

		if( nBytes != m_nBytes )
		{
			cleanup();

			m_nBytes = nBytes;

			m_pOutputVBO = GLVertexBufferObject::fromUnsignedByteArray
			(
				m_aubInputRGB.data(),
				nBytes,
				nBytes / 3,
				GLBufferObject::USAGE_STREAM_DRAW
			);

			m_pBufferObjectOutputPort->pushData( KernelPortData( m_pOutputVBO ) );
		}
	}
	else
	{
		// there's only one input port
		m_qvfInputRGB = m_pInputRGBInputPort->pullData().getFloatArrayData();
		int nBytes = m_qvfInputRGB.size() * sizeof( float );

		if( nBytes != m_nBytes )
		{
			cleanup();	

			m_nBytes = nBytes;

			m_pOutputVBO = GLVertexBufferObject::fromFloatArray
			(
				m_qvfInputRGB.data(),
				m_qvfInputRGB.size(),
				m_qvfInputRGB.size() / 4,
				GLBufferObject::USAGE_STREAM_DRAW
			);

			m_pBufferObjectOutputPort->pushData( KernelPortData( m_pOutputVBO ) );
		}
	}

	m_bReallocationNeeded = false;
}
