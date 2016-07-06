#include "GridPaintingKernel.h"

#include <cassert>
#include <cfloat>
#include <Cg/CgProgramManager.h>
#include <Cg/CgProgramWrapper.h>
#include <Cg/CgShared.h>
#include <Cg/CgUtilities.h>
#include <cmath>
#include <common/ArrayWithLength.h>
#include <common/ArrayUtils.h>
#include <color/ColorUtils.h>
#include <GL/GLFramebufferObject.h>
#include <GL/GLShared.h>
#include <GL/GLTextureRectangle.h>
#include <GL/GLUtilities.h>

#include <QCursor>

#include <math/Arithmetic.h>
#include <math/MathUtils.h>

#include "AppData.h"
#include "OutputWidget.h"

//////////////////////////////////////////////////////////////////////////
// Public
//////////////////////////////////////////////////////////////////////////

// static
void GridPaintingKernel::initializeCg()
{
	QString cgPrefix = AppData::getInstance()->getCgPathPrefix();
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	CGprofile latestCgFragmentProfile = CgShared::getInstance()->getLatestFragmentProfile();

	pProgramManager->loadProgramFromFile( "GP_sliceGridAndRotateColors", cgPrefix + "GridPainting.cg",
		"sliceGridAndRotateColors", latestCgFragmentProfile, NULL );

	s_bCgInitialized = true;
}

// static
GridPaintingKernel* GridPaintingKernel::create( QString args )
{
	if( !s_bCgInitialized )
	{
		initializeCg();
	}

	GridPaintingKernel* pInstance = new GridPaintingKernel;
	pInstance->initializeGL();
	return pInstance;
}

// virtual
GridPaintingKernel::~GridPaintingKernel()
{
	cleanup();
	delete m_pGridTexture;
}

// virtual
bool GridPaintingKernel::isInputComplete()
{	
	return true;
}

// virtual
void GridPaintingKernel::makeDirty( QString inputPortName )
{
	m_bReallocationNeeded = true;
	m_pOutputTextureOutputPort->makeDirty();
}

// virtual
void GridPaintingKernel::compute( QString outputPortName )
{
	if( m_bReallocationNeeded )
	{
		reallocate();		
	}	

	sliceGridAndRotateColorsPass();
}

//////////////////////////////////////////////////////////////////////////
// public slots
//////////////////////////////////////////////////////////////////////////

void GridPaintingKernel::handleMousePressed( int x, int y, int button )
{
	// QApplication::setOverrideCursor( QCursor( Qt:CrossCursor ) );
	AppData::getInstance()->getOutputWidget()->setCursor( Qt::CrossCursor );
	// TODO: splat gaussian

	m_iLastPressedButton = button;
	if( m_iLastPressedButton != 0 )
	{
		// read rgb( x, y )
		float clickedPixelRGB[3];
		m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pInputTexture );
		glReadPixels( x, y, 1, 1, GL_RGB, GL_FLOAT, clickedPixelRGB );
		m_pFBO->detachTexture( GL_COLOR_ATTACHMENT0_EXT );
		float luminance = ColorUtils::rgb2luminance( clickedPixelRGB );

		// compute z( rgb( x, y ) )
		int gridX = Arithmetic::roundToInt( x / m_fSigmaSpatial + m_iPaddingXY );
		int gridY = Arithmetic::roundToInt( y / m_fSigmaSpatial + m_iPaddingXY );
		float gridZ = luminance / m_fSigmaRange + m_iPaddingZ;
		m_iMousePressedGridZ0 = Arithmetic::floorToInt( gridZ );
		m_iMousePressedGridZ1 = m_iMousePressedGridZ0 + 1;

		float gridValue;
		if( m_iLastPressedButton == 1 )
		{
			gridValue = 1;
		}
		else
		{
			gridValue = 0;
		}

		m_pGridTexture->setFloat1SubData( &gridValue, m_iMousePressedGridZ0 * m_iGridWidth + gridX, gridY, 1, 1 );
		m_pGridTexture->setFloat1SubData( &gridValue, m_iMousePressedGridZ1 * m_iGridWidth + gridX, gridY, 1, 1 );

		m_pOutputTextureOutputPort->makeDirty();
	}
}

void GridPaintingKernel::handleMouseMoved( int x, int y )
{
	if( m_iLastPressedButton != 0 )
	{
		// compute z( rgb( x, y ) )
		int gridX = Arithmetic::roundToInt( x / m_fSigmaSpatial + m_iPaddingXY );
		int gridY = Arithmetic::roundToInt( y / m_fSigmaSpatial + m_iPaddingXY );

		float gridValue;
		if( m_iLastPressedButton == 1 )
		{
			gridValue = 1;
		}
		else
		{
			gridValue = 0;
		}

		m_pGridTexture->setFloat1SubData( &gridValue, m_iMousePressedGridZ0 * m_iGridWidth + gridX, gridY, 1, 1 );
		m_pGridTexture->setFloat1SubData( &gridValue, m_iMousePressedGridZ1 * m_iGridWidth + gridX, gridY, 1, 1 );

		m_pOutputTextureOutputPort->makeDirty();
	}
}

void GridPaintingKernel::handleMouseReleased( int x, int y, int button )
{
	m_iLastPressedButton = 0;

	AppData::getInstance()->getOutputWidget()->unsetCursor();
}

//////////////////////////////////////////////////////////////////////////
// protected
//////////////////////////////////////////////////////////////////////////

// virtual
void GridPaintingKernel::initializeGL()
{
	GPUKernel::initializeGL();
	initializeCgPrograms();

	m_iGridTextureWidth = 4096;
	m_iGridTextureHeight = 256;

	ArrayWithLength< float > float1Zeroes = ArrayUtils::createFloatArray( m_iGridTextureWidth * m_iGridTextureHeight, 0 );
	m_pGridTexture = GLTextureRectangle::createFloat1Texture( m_iGridTextureWidth, m_iGridTextureHeight, 32, float1Zeroes );
	float1Zeroes.destroy();
}

// virtual
void GridPaintingKernel::initializePorts()
{
	// -- Input Ports --
	m_pInputTextureInputPort = addInputPort( "inputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );

	// float parameters
	m_pSigmaSpatialInputPort = addInputPort( "sigmaSpatial", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pSigmaSpatialInputPort->setFloatMinMaxDelta( 16.f, 5.f, 50.f, 1.f );

	m_pSigmaRangeInputPort = addInputPort( "sigmaRange", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pSigmaRangeInputPort->setFloatMinMaxDelta( 0.1, 0.01f, 1.f, 0.01f );

	// -- Output Ports --
	m_pOutputTextureOutputPort = addOutputPort( "outputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

// static
bool GridPaintingKernel::s_bCgInitialized = false;

GridPaintingKernel::GridPaintingKernel() :

	GPUKernel( "GridPainting" ),
	m_pFBO( GLShared::getInstance()->getSharedFramebufferObject() ),

	m_iPaddingXY( 1 ),
	m_iPaddingZ( 1 ),

	m_iInputWidth( -1 ), // invalid
	m_iInputHeight( -1 ), // invalid

	m_pGridTexture( NULL ),
	m_pOutputTexture( NULL ),

	m_bReallocationNeeded( true )
{

}

void GridPaintingKernel::initializeCgPrograms()
{
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();

	/////////////////////////////////////////////////////////////////////

	m_pSliceGridAndRotateColorsProgram = pProgramManager->getNamedProgram( "GP_sliceGridAndRotateColors" );
	m_cgp_SGRC_inputSampler = m_pSliceGridAndRotateColorsProgram->getNamedParameter( "inputSampler" );
	m_cgp_SGRC_gridSampler = m_pSliceGridAndRotateColorsProgram->getNamedParameter( "gridSampler" );
	m_cgp_SGRC_f3_rcpSigma = m_pSliceGridAndRotateColorsProgram->getNamedParameter( "rcpSigma" );
	m_cgp_SGRC_f3_gridSize = m_pSliceGridAndRotateColorsProgram->getNamedParameter( "gridSize" );
}

void GridPaintingKernel::cleanup()
{
	if( m_pOutputTexture != NULL )
	{
		delete m_pOutputTexture;
		m_pOutputTexture = NULL;
	}
}

void GridPaintingKernel::reallocate()
{
	// retrieve inputs
	float sigmaSpatial = m_pSigmaSpatialInputPort->pullData().getFloatData();
	float sigmaRange = m_pSigmaRangeInputPort->pullData().getFloatData();
	m_pInputTexture = m_pInputTextureInputPort->pullData().getGLTextureRectangleData();

	bool sizeChanged = false;
	if( m_iInputWidth != m_pInputTexture->getWidth() )
	{
		m_iInputWidth = m_pInputTexture->getWidth();
		sizeChanged = true;
	}
	if( m_iInputHeight != m_pInputTexture->getHeight() )
	{
		m_iInputHeight = m_pInputTexture->getHeight();
		sizeChanged = true;
	}

	if( sizeChanged )
	{
		cleanup();

		int nBits = AppData::getInstance()->getTextureNumBits();
		m_pOutputTexture = GLTextureRectangle::createFloat4Texture( m_iInputWidth, m_iInputHeight, nBits );
		m_pOutputTextureOutputPort->pushData( KernelPortData( m_pOutputTexture ) );		
	}

	bool sigmaChanged = false;
	if( m_fSigmaSpatial != sigmaSpatial )
	{
		m_fSigmaSpatial = sigmaSpatial;
		sigmaChanged = true;
	}
	if( m_fSigmaRange != sigmaRange )
	{
		m_fSigmaRange = sigmaRange;
		sigmaChanged = true;
	}
	
	if( sigmaChanged )
	{
		m_iGridWidth = static_cast< int >( ( m_iInputWidth - 1 ) / m_fSigmaSpatial ) + 1 + 2 * m_iPaddingXY;
		m_iGridHeight = static_cast< int >( ( m_iInputHeight - 1 ) / m_fSigmaSpatial ) + 1 + 2 * m_iPaddingXY;
		m_iGridDepth = static_cast< int >( 1.0f / m_fSigmaRange ) + 1 + 2 * m_iPaddingZ;

		clearGridPass();
	}

	m_bReallocationNeeded = false;
}

void GridPaintingKernel::clearGridPass()
{
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pGridTexture );
	glClear( GL_COLOR_BUFFER_BIT );
}

void GridPaintingKernel::sliceGridAndRotateColorsPass()
{
	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pOutputTexture );

	// setup input
	cgGLSetTextureParameter( m_cgp_SGRC_inputSampler,
		m_pInputTexture->getTextureId() );
	m_pGridTexture->setFilterMode( GLTexture::FILTER_MODE_LINEAR, GLTexture::FILTER_MODE_LINEAR );
	cgGLSetTextureParameter( m_cgp_SGRC_gridSampler,
		m_pGridTexture->getTextureId() );

	// setup uniforms
	cgGLSetParameter3f( m_cgp_SGRC_f3_rcpSigma,
		1.f / m_fSigmaSpatial, 1.f / m_fSigmaSpatial, 1.f / m_fSigmaRange );
	cgGLSetParameter3f( m_cgp_SGRC_f3_gridSize,
		m_iGridWidth, m_iGridHeight, m_iGridDepth );

	// render
	glClear( GL_COLOR_BUFFER_BIT );
	GLUtilities::setupOrthoCamera( m_iInputWidth, m_iInputHeight );
	m_pSliceGridAndRotateColorsProgram->bind();
	GLUtilities::drawQuad( m_iInputWidth, m_iInputHeight );

	m_pGridTexture->setFilterMode( GLTexture::FILTER_MODE_NEAREST, GLTexture::FILTER_MODE_NEAREST );
}
