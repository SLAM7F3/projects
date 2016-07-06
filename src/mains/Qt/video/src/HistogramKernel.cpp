#include "HistogramKernel.h"

#include <cassert>
#include <Cg/CgUtilities.h>
#include <Cg/CgProgramManager.h>
#include <Cg/CgProgramWrapper.h>
#include <Cg/CgShared.h>
#include <GL/GLFramebufferObject.h>
#include <GL/GLShared.h>
#include <GL/GLTextureRectangle.h>
#include <GL/GLUtilities.h>
#include <GL/GLBufferObject.h>
#include <GL/GLVertexBufferObject.h>

#include "AppData.h"

//////////////////////////////////////////////////////////////////////////
// Public
//////////////////////////////////////////////////////////////////////////

// static
void HistogramKernel::initializeCg()
{
	QString cgPrefix = AppData::getInstance()->getCgPathPrefix();
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	CGprofile latestCgVertexProfile = CgShared::getInstance()->getLatestVertexProfile();
	CGprofile latestCgFragmentProfile = CgShared::getInstance()->getLatestFragmentProfile();

	pProgramManager->loadProgramFromFile( "HISTOGRAM_scatterHistogramVertexStandard", cgPrefix + "Histogram.cg",
		"scatterHistogramVertexStandard", latestCgVertexProfile, NULL );

	pProgramManager->loadProgramFromFile( "HISTOGRAM_scatterHistogramVertexAbsoluteValue", cgPrefix + "Histogram.cg",
		"scatterHistogramVertexAbsoluteValue", latestCgVertexProfile, NULL );

	pProgramManager->loadProgramFromFile( "HISTOGRAM_scatterHistogramFragment", cgPrefix + "Histogram.cg",
		"scatterHistogramFragment", latestCgFragmentProfile, NULL );

	s_bCgInitialized = true;
}

// static
HistogramKernel* HistogramKernel::create( QString args )
{
	if( !s_bCgInitialized )
	{
		initializeCg();
	}

	HistogramKernel* pInstance = new HistogramKernel( args );
	pInstance->initializeGL();
	return pInstance;
}

// virtual
HistogramKernel::~HistogramKernel()
{	
	deleteOutputTexture();
}

// virtual
bool HistogramKernel::isInputComplete()
{
	return true;
}

// virtual
void HistogramKernel::makeDirty( QString inputPortName )
{
	if( inputPortName == "nBins" || inputPortName == "inputTexture" )
	{
		m_bReallocationNeeded = true;
	}
	m_pOutputPort->makeDirty();
}

// virtual
void HistogramKernel::compute( QString outputPortName )
{
	if( m_bReallocationNeeded )
	{
		reallocate();
	}

	// retrieve inputs

	// setup outputs
	cgGLEnableProfile( CgShared::getInstance()->getLatestVertexProfile() );
	glEnable( GL_BLEND );
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pOutputTexture );

	// setup inputs
	cgGLSetTextureParameter( m_cgp_V_inputLabASampler, m_pInputTexture->getTextureId() );	

	// setup uniforms
	CgUtilities::setupOrthoCamera( m_nBins, 1, m_cgp_V_mvp );
	cgGLSetParameter1f( m_cgp_V_fNumBins, m_nBins );
	cgGLSetParameter1f( m_cgp_V_fReciprocalNumPixels, 1.f / m_pXYCoordinateVBO->getNumElements() );

	glClear( GL_COLOR_BUFFER_BIT );
	m_pVertexProgram->bind();
	m_pFragmentProgram->bind();

	m_pXYCoordinateVBO->bind( GLBufferObject::TARGET_ARRAY_BUFFER );
	glVertexPointer( 2, GL_SHORT, 0, NULL );
	glDrawArrays( GL_POINTS, 0, m_pXYCoordinateVBO->getNumElements() );

	glDisable( GL_BLEND );
	cgGLDisableProfile( CgShared::getInstance()->getLatestVertexProfile() );
}

//////////////////////////////////////////////////////////////////////////
// protected
//////////////////////////////////////////////////////////////////////////

// virtual
void HistogramKernel::initializeGL()
{
	GPUKernel::initializeGL();
	initializeCgPrograms();
}

// virtual					
void HistogramKernel::initializePorts()
{
	m_pNumBinsInputPort = addInputPort( "nBins", KERNEL_PORT_DATA_TYPE_INT );
	m_pNumBinsInputPort->setIntMinMaxDelta( 256, 10, 1024, 1 );

	m_pInputTextureInputPort = addInputPort( "inputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );

	m_pOutputPort = addOutputPort( "outputTexture",	KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

// static
bool HistogramKernel::s_bCgInitialized = false;

HistogramKernel::HistogramKernel( QString args ) :

	GPUKernel( "Histogram" ),
	m_pFBO( GLShared::getInstance()->getSharedFramebufferObject() ),

	m_nBins( -1 ), // invalid
	m_iInputWidth( -1 ), // invalid
	m_iInputHeight( -1 ), // invalid

	m_bReallocationNeeded( true ),

	// outputs
	m_pOutputTexture( NULL )
{
	if( args == "standard" )
	{
		m_bIsAbsoluteValue = false;
	}
	else if( args == "absolutevalue" )
	{
		m_bIsAbsoluteValue = true;
	}
	else
	{
		fprintf( stderr, "Histogram: args must be \"standard\" or \"absolutevalue\"\n" );
		exit( -1 ); // exit, not assert, too dangerous
	}
}

void HistogramKernel::deleteOutputTexture()
{
	if( m_pOutputTexture != NULL )
	{
		delete m_pOutputTexture;
		m_pOutputTexture = NULL;
	}
}

void HistogramKernel::reallocate()
{
	bool sizeChanged = false;

	m_pInputTexture = m_pInputTextureInputPort->pullData().getGLTextureRectangleData();
	int inputWidth = m_pInputTexture->getWidth();
	int inputHeight = m_pInputTexture->getHeight();

	if( m_iInputWidth != inputWidth )
	{
		m_iInputWidth = inputWidth;
		sizeChanged = true;
	}
	if( m_iInputHeight != inputHeight )
	{
		m_iInputHeight = inputHeight;
		sizeChanged = true;
	}

	int nBins = m_pNumBinsInputPort->pullData().getIntData();
	if( nBins != m_nBins )
	{
		m_nBins = nBins;
		deleteOutputTexture();

		int nBits = AppData::getInstance()->getTextureNumBits();		

		m_pOutputTexture = GLTextureRectangle::createFloat1Texture( m_nBins, 1, nBits );
		m_pOutputPort->pushData( KernelPortData( m_pOutputTexture ) );		
	}

	if( sizeChanged )
	{
		m_nPixels = m_iInputWidth * m_iInputHeight;

#if 0

#if 0
		int index = 0;
		GLshort* xyCoords = new GLshort[ 2 * m_nPixels ];

		for( int y = 0; y < m_iInputHeight; ++y )
		{
			for( int x = 0; x < m_iInputWidth; ++x )
			{
				xyCoords[ index ] = x;
				xyCoords[ index + 1 ] = y;

				index += 2;
			}
		}

		m_pXYCoordinateVBO = GLVertexBufferObject::fromShortArray( xyCoords, 2 * m_nPixels, m_nPixels );

#else

		// cheat: drop 3/4 pixels

		int index = 0;
		GLshort* xyCoords = new GLshort[ 2 * m_nPixels / 4 ];

		for( int y = 0; y < m_iInputHeight; y += 2 )
		{
			for( int x = 0; x < m_iInputWidth; x += 2 )
			{
				xyCoords[ index ] = x;
				xyCoords[ index + 1 ] = y;

				index += 2;
			}
		}

		m_pXYCoordinateVBO = GLVertexBufferObject::fromShortArray( xyCoords, 2 * m_nPixels / 4, m_nPixels / 4 );

#endif

		delete[] xyCoords;
#endif

		m_pXYCoordinateVBO = GLShared::getInstance()->getSharedXYCoordinateVBO
		(
			m_iInputWidth, m_iInputHeight
		);
	}

	m_bReallocationNeeded = false;
}

void HistogramKernel::initializeCgPrograms()
{
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();

	if( m_bIsAbsoluteValue )
	{
		m_pVertexProgram = pProgramManager->getNamedProgram( "HISTOGRAM_scatterHistogramVertexAbsoluteValue" );
	}
	else
	{
		m_pVertexProgram = pProgramManager->getNamedProgram( "HISTOGRAM_scatterHistogramVertexStandard" );		
	}

	m_cgp_V_mvp = m_pVertexProgram->getNamedParameter( "mvp" );
	m_cgp_V_fNumBins = m_pVertexProgram->getNamedParameter( "nBins" );
	m_cgp_V_fReciprocalNumPixels = m_pVertexProgram->getNamedParameter( "reciprocalNumPixels" );
	m_cgp_V_inputLabASampler = m_pVertexProgram->getNamedParameter( "inputLabASampler" );

	m_pFragmentProgram = pProgramManager->getNamedProgram( "HISTOGRAM_scatterHistogramFragment" );
}
