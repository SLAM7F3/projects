#include "RGB2TextureKernel.h"

#include <cassert>
#include <GL/GLTextureRectangle.h>
#include <GL/GLUtilities.h>

#include "AppData.h"

// static
RGB2TextureKernel* RGB2TextureKernel::create( QString args )
{
	RGB2TextureKernel* pInstance = new RGB2TextureKernel( args );
	pInstance->initializeGL();
	return pInstance;
}

// virtual
RGB2TextureKernel::~RGB2TextureKernel()
{
	cleanup();
}

// virtual
bool RGB2TextureKernel::isInputComplete()
{
	return m_pInputRGBInputPort->isConnected();
}

// virtual
void RGB2TextureKernel::makeDirty( QString inputPortName )
{
	// 1 in 1 out
	m_bReallocationNeeded = true;
	m_pOutputTextureOutputPort->makeDirty();	
}

// virtual
void RGB2TextureKernel::compute( QString outputPortName )
{
	if( m_bReallocationNeeded )
	{
		reallocate();		
	}

	if( m_bIsUnsignedByte )
	{
		m_pOutputTexture->setUnsignedByte3Data( m_aubInputRGB );

#if 0
		// TODO: remove me
		ubyte r = m_aubInputRGB[ 461440 ];
		ubyte g = m_aubInputRGB[ 461441 ];
		ubyte b = m_aubInputRGB[ 461442 ];

		printf( "middle rgb = %d, %d, %d\n", r, g, b );
#endif
	}
	else
	{
		m_pOutputTexture->setFloat4Data( m_qvfInputRGB.constData() );
	}
}

//////////////////////////////////////////////////////////////////////////
// protected
//////////////////////////////////////////////////////////////////////////

// virtual
void RGB2TextureKernel::initializeGL()
{
	GPUKernel::initializeGL();
}

// virtual
void RGB2TextureKernel::initializePorts()
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
	m_pSizeInputPort = addInputPort( "size", KERNEL_PORT_DATA_TYPE_SIZE_2D );

	// -- Output Ports --
	m_pOutputTextureOutputPort = addOutputPort( "outputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

RGB2TextureKernel::RGB2TextureKernel( QString args ) :

	GPUKernel( "RGB2Texture" ),

	m_bReallocationNeeded( true ),

	m_iWidth( -1 ), // invalid so it will be != on first comparison
	m_iHeight( -1 ), // invalid so it will be != on first comparison

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
		fprintf( stderr, "RGB2TextureKernel: args must be \"ubyte\" or \"float\"\n" );
		exit( -1 );
	}
}

void RGB2TextureKernel::cleanup()
{
	if( m_pOutputTexture != NULL )
	{
		delete m_pOutputTexture;
		m_pOutputTexture = NULL;
	}
}

void RGB2TextureKernel::reallocate()
{
	QSize size = m_pSizeInputPort->pullData().getSize2DData();
	KernelPortData inputRGB = m_pInputRGBInputPort->pullData();
	if( m_bIsUnsignedByte )
	{
		m_aubInputRGB = inputRGB.getUnsignedByteArrayData();
	}
	else
	{
		m_qvfInputRGB = inputRGB.getFloatArrayData();
	}	

	if( size != m_qSize )
	{
		cleanup();

		m_qSize = size;
		m_iWidth = size.width();
		m_iHeight = size.height();

		if( m_bIsUnsignedByte )
		{
			m_pOutputTexture = GLTextureRectangle::createUnsignedByte3Texture( m_iWidth, m_iHeight );
		}
		else
		{			
			int nBits = AppData::getInstance()->getTextureNumBits();
			m_pOutputTexture = GLTextureRectangle::createFloat4Texture( m_iWidth, m_iHeight, nBits );
		}
		m_pOutputTextureOutputPort->pushData( KernelPortData( m_pOutputTexture ) );
	}

	m_bReallocationNeeded = false;
}
