#include "InteractiveToneMapKernel.h"

#include <cassert>
#include <cfloat>
#include <Cg/CgProgramManager.h>
#include <Cg/CgProgramWrapper.h>
#include <Cg/CgShared.h>
#include <Cg/CgUtilities.h>
#include <cmath>
#include <color/ColorUtils.h>
#include <common/ArrayWithLength.h>
#include <GL/GLFramebufferObject.h>
#include <GL/GLShared.h>
#include <GL/GLTextureRectangle.h>
#include <GL/GLUtilities.h>
#include <math/Arithmetic.h>
#include <math/MathUtils.h>

#include "AppData.h"

//////////////////////////////////////////////////////////////////////////
// Public
//////////////////////////////////////////////////////////////////////////

// static
void InteractiveToneMapKernel::initializeCg()
{
	QString cgPrefix = AppData::getInstance()->getCgPathPrefix();
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	CGprofile latestCgFragmentProfile = CgShared::getInstance()->getLatestFragmentProfile();

	pProgramManager->loadProgramFromFile( "ITM_initializeGrid", cgPrefix + "InteractiveToneMapping.cg",
		"initializeGrid", latestCgFragmentProfile, NULL );

	pProgramManager->loadProgramFromFile( "ITM_sliceGridAndToneMap", cgPrefix + "InteractiveToneMapping.cg",
		"sliceGridAndToneMap", latestCgFragmentProfile, NULL );

	s_bCgInitialized = true;
}

// static
InteractiveToneMapKernel* InteractiveToneMapKernel::create( QString args )
{
	if( !s_bCgInitialized )
	{
		initializeCg();
	}

	InteractiveToneMapKernel* pInstance = new InteractiveToneMapKernel;
	pInstance->initializeGL();
	return pInstance;
}

// virtual
InteractiveToneMapKernel::~InteractiveToneMapKernel()
{
	cleanup();

	if( m_pGridTexture != NULL )
	{
		delete m_pGridTexture;
		m_pGridTexture = NULL;
	}
}

// virtual
bool InteractiveToneMapKernel::isInputComplete()
{	
	return true;
}

// virtual
void InteractiveToneMapKernel::makeDirty( QString inputPortName )
{
	m_bReallocationNeeded = true;
	m_pOutputTextureOutputPort->makeDirty();
}

// virtual
void InteractiveToneMapKernel::compute( QString outputPortName )
{
	if( m_bReallocationNeeded )
	{
		reallocate();		
	}
	
	// dodgeBurnPass();
	sliceGridAndToneMapPass();
}

//////////////////////////////////////////////////////////////////////////
// public slots
//////////////////////////////////////////////////////////////////////////

void InteractiveToneMapKernel::handleMousePressed( int x, int y, int button )
{
	m_iLastPressedButton = button;
	if( m_iLastPressedButton != 0 )
	{
		// retrieve inputs
		// read the delta factor
		m_pBaseTexture = m_pBaseTextureInputPort->pullData().getGLTextureRectangleData();		
		m_fDodgeBurnDelta = m_pDodgeBurnDeltaInputPort->pullData().getFloatData();

		// compute what grid xyz they clicked on
		int gridX;
		int gridY;
		float gridZ;
		getGridXYZFromClick( x, y, &gridX, &gridY, &gridZ );

		printf( "gridxyz = %d, %d, %f\n", gridX, gridY, gridZ );

#if 1
		m_iMousePressedGridZ0 = Arithmetic::floorToInt( gridZ );
		m_iMousePressedGridZ1 = m_iMousePressedGridZ0 + 1;
		float alphaZ = gridZ - m_iMousePressedGridZ0;

		// retrieve the value g( x, y, z )
		float gridValueZ0;
		float gridValueZ1;
		readGridAtTwoZs( gridX, gridY, m_iMousePressedGridZ0, m_iMousePressedGridZ1, &gridValueZ0, &gridValueZ1 );
		
		if( m_iLastPressedButton == 1 )
		{
			gridValueZ0 += m_fDodgeBurnDelta;
			gridValueZ1 += m_fDodgeBurnDelta;
		}
		else
		{
			gridValueZ0 -= m_fDodgeBurnDelta;
			gridValueZ1 -= m_fDodgeBurnDelta;
		}

		// printf( "new grid values = %f, %f\n", gridValueZ0, gridValueZ1 );

		m_pGridTexture->setFloat1SubData( &gridValueZ0, m_iMousePressedGridZ0 * m_iGridWidth + gridX, gridY, 1, 1 );
		m_pGridTexture->setFloat1SubData( &gridValueZ1, m_iMousePressedGridZ1 * m_iGridWidth + gridX, gridY, 1, 1 );

		m_pOutputTextureOutputPort->makeDirty();
#endif
	}
}

void InteractiveToneMapKernel::handleMouseMoved( int x, int y )
{
	if( m_iLastPressedButton != 0 )
	{
#if 0
		float gridZ;
		getGridXYZFromClick( x, y, &gridX, &gridY, &gridZ );
		printf( "gridxyz = %d, %d, %f\n", gridX, gridY, gridZ );

#else
		int gridX;
		int gridY;
		getGridXYFromClick( x, y, &gridX, &gridY );

		bool gridXYChanged = false;
		if( gridX != m_iLastGridX )
		{
			m_iLastGridX = gridX;
			gridXYChanged = true;
		}
		if( gridY != m_iLastGridY )
		{
			m_iLastGridY = gridY;
			gridXYChanged = true;
		}

		if( gridXYChanged )
		{
			// retrieve the value g( x, y, z )
			float gridValueZ0;
			float gridValueZ1;
			readGridAtTwoZs( gridX, gridY, m_iMousePressedGridZ0, m_iMousePressedGridZ1, &gridValueZ0, &gridValueZ1 );

			if( m_iLastPressedButton == 1 )
			{
				gridValueZ0 += m_fDodgeBurnDelta;
				gridValueZ1 += m_fDodgeBurnDelta;
			}
			else
			{
				gridValueZ0 -= m_fDodgeBurnDelta;
				gridValueZ1 -= m_fDodgeBurnDelta;
			}

			printf( "new grid values = %f, %f\n", gridValueZ0, gridValueZ1 );

			m_pGridTexture->setFloat1SubData( &gridValueZ0, m_iMousePressedGridZ0 * m_iGridWidth + gridX, gridY, 1, 1 );
			m_pGridTexture->setFloat1SubData( &gridValueZ1, m_iMousePressedGridZ1 * m_iGridWidth + gridX, gridY, 1, 1 );

			m_pOutputTextureOutputPort->makeDirty();
		}
#endif
	}
}

void InteractiveToneMapKernel::handleMouseReleased( int x, int y, int button )
{
	m_iLastPressedButton = 0;
}

//////////////////////////////////////////////////////////////////////////
// protected
//////////////////////////////////////////////////////////////////////////

// virtual
void InteractiveToneMapKernel::initializeGL()
{
	GPUKernel::initializeGL();
	initializeCgPrograms();

	m_iGridTextureWidth = 8192;
	m_iGridTextureHeight = 256;

	m_pGridTexture = GLTextureRectangle::createFloat1Texture( m_iGridTextureWidth, m_iGridTextureHeight, 32 );
}

// virtual
void InteractiveToneMapKernel::initializePorts()
{
	// -- Input Ports --
	m_pInputTextureInputPort = addInputPort( "inputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
	m_pBaseTextureInputPort = addInputPort( "baseTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );

	// float parameters
	m_pLogBaseMinInputPort = addInputPort( "logBaseMin", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pLogBaseMaxInputPort = addInputPort( "logBaseMax", KERNEL_PORT_DATA_TYPE_FLOAT );
	
	m_pTargetContrastInputPort = addInputPort( "targetContrast", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pTargetContrastInputPort->setFloatMinMaxDelta( 5.f, 2.f, 20.f, 1.f );
	
	m_pSigmaSpatialInputPort = addInputPort( "sigmaSpatial", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pSigmaSpatialInputPort->setFloatMinMaxDelta( 32, 5.f, 128.f, 1.f );

	m_pSigmaRangeInputPort = addInputPort( "sigmaRange", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pSigmaRangeInputPort->setFloatMinMaxDelta( 0.1, 0.01f, 1.f, ( 1.f - 0.01f ) / 200.f );

	m_pDodgeBurnDeltaInputPort = addInputPort( "delta", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pDodgeBurnDeltaInputPort->setFloatMinMaxDelta( 0.01f, 0.f, 5.f, 0.01f );

	// -- Output Ports --
	m_pOutputTextureOutputPort = addOutputPort( "outputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

// static
bool InteractiveToneMapKernel::s_bCgInitialized = false;

InteractiveToneMapKernel::InteractiveToneMapKernel() :

	GPUKernel( "InteractiveToneMap" ),
	m_pFBO( GLShared::getInstance()->getSharedFramebufferObject() ),

	m_iPaddingXY( 1 ),
	m_iPaddingZ( 1 ),

	m_iInputWidth( -1 ),
	m_iInputHeight( -1 ),
	m_fLogBaseMin( FLT_MAX ),
	m_fLogBaseMax( FLT_MIN ),

	m_iOutputBaseRemappingGridIndex( 0 ),
	
	m_pGridTexture( NULL ),
	m_pOutputTexture( NULL ),

	m_bReallocationNeeded( true ),
	m_iLastPressedButton( 0 ),
	m_iLastGridX( -1 ), // invalid
	m_iLastGridY( -1 ) // invalid
{

}

void InteractiveToneMapKernel::initializeCgPrograms()
{
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();

	/////////////////////////////////////////////////////////////////////	

	m_pInitializeGridProgram = pProgramManager->getNamedProgram( "ITM_initializeGrid" );
	m_cgp_IG_f2LogLuminanceMinMax = m_pInitializeGridProgram->getNamedParameter( "logLuminanceMinMax" );
	m_cgp_IG_fCompressionFactor = m_pInitializeGridProgram->getNamedParameter( "compressionFactor" );
	m_cgp_IG_f3GridWidthHeightDepth = m_pInitializeGridProgram->getNamedParameter( "gridWidthHeightDepth" );

	/////////////////////////////////////////////////////////////////////

	m_pSliceGridAndToneMapProgram = pProgramManager->getNamedProgram( "ITM_sliceGridAndToneMap" );
	m_cgp_SGTM_inputSampler = m_pSliceGridAndToneMapProgram->getNamedParameter( "inputSampler" );
	m_cgp_SGTM_logBaseSampler = m_pSliceGridAndToneMapProgram->getNamedParameter( "logBaseSampler" );
	m_cgp_SGTM_baseGridSampler = m_pSliceGridAndToneMapProgram->getNamedParameter( "baseGridSampler" );
	m_cgp_SGTM_f3_rcpSigma = m_pSliceGridAndToneMapProgram->getNamedParameter( "rcpSigma" );
	m_cgp_SGTM_f3_gridSize = m_pSliceGridAndToneMapProgram->getNamedParameter( "gridSize" );
	m_cgp_SGTM_f2LogBaseMinMax = m_pSliceGridAndToneMapProgram->getNamedParameter( "logBaseMinMax" );
	m_cgp_SGTM_fLogAbsoluteScale = m_pSliceGridAndToneMapProgram->getNamedParameter( "logAbsoluteScale" );
}

void InteractiveToneMapKernel::cleanup()
{
	if( m_pOutputTexture != NULL )
	{
		delete m_pOutputTexture;
		m_pOutputTexture = NULL;
	}	
}

void InteractiveToneMapKernel::reallocate()
{
	// check sizes
	m_pInputTexture = m_pInputTextureInputPort->pullData().getGLTextureRectangleData();
	m_pBaseTexture = m_pBaseTextureInputPort->pullData().getGLTextureRectangleData();

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

	assert( m_iInputWidth == m_pBaseTexture->getWidth() );
	assert( m_iInputHeight == m_pBaseTexture->getHeight() );

	// check min and max
	float logBaseMin = m_pLogBaseMinInputPort->pullData().getFloatData();
	float logBaseMax = m_pLogBaseMaxInputPort->pullData().getFloatData();

	bool minMaxChanged = false;
	if( logBaseMin != m_fLogBaseMin )
	{
		m_fLogBaseMin = logBaseMin;
		minMaxChanged = true;
	}
	if( logBaseMax != m_fLogBaseMax )
	{
		m_fLogBaseMax = logBaseMax;
		minMaxChanged = true;
	}

	// check if target contrast changed
	bool targetContrastChanged = false;
	float targetContrast = m_pTargetContrastInputPort->pullData().getFloatData();
	if( targetContrast != m_fTargetContrast )
	{
		m_fTargetContrast = targetContrast;
		targetContrastChanged = true;
	}

	bool sigmaChanged = false;
	float sigmaSpatial = m_pSigmaSpatialInputPort->pullData().getFloatData();
	float sigmaRange = m_pSigmaRangeInputPort->pullData().getFloatData();
	if( sigmaSpatial != m_fSigmaSpatial )
	{
		m_fSigmaSpatial = sigmaSpatial;
		sigmaChanged = true;
	}
	if( sigmaRange != m_fSigmaRange )
	{
		m_fSigmaRange = sigmaRange;
		sigmaChanged = true;
	}

	if( sizeChanged || minMaxChanged || targetContrastChanged || sigmaChanged )
	{
		cleanup();

		m_iGridWidth = static_cast< int >( ( m_iInputWidth - 1 ) / m_fSigmaSpatial ) + 1 + 2 * m_iPaddingXY;
		m_iGridHeight = static_cast< int >( ( m_iInputHeight - 1 ) / m_fSigmaSpatial ) + 1 + 2 * m_iPaddingXY;
		m_iGridDepth = static_cast< int >( 1.f / m_fSigmaRange ) + 1 + 2 * m_iPaddingZ;

		assert( m_iGridWidth * m_iGridDepth <= m_iGridTextureWidth );
		assert( m_iGridHeight <= m_iGridTextureHeight );

		m_fCompressionFactor = log10( m_fTargetContrast ) / ( m_fLogBaseMax - m_fLogBaseMin );

		int nBits = AppData::getInstance()->getTextureNumBits();
		m_pOutputTexture = GLTextureRectangle::createFloat4Texture( m_iInputWidth, m_iInputHeight, nBits );
		m_pOutputTextureOutputPort->pushData( KernelPortData( m_pOutputTexture ) );		

		initializeGridPass();
	}

	m_bReallocationNeeded = false;
}

void InteractiveToneMapKernel::initializeGridPass()
{
	// setup output	
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pGridTexture );

	// setup uniforms
	// TODO: this doesn't matter!
	cgGLSetParameter2f( m_cgp_IG_f2LogLuminanceMinMax,
		m_fLogBaseMin, m_fLogBaseMax );	
	cgGLSetParameter1f( m_cgp_IG_fCompressionFactor,
		m_fCompressionFactor );
	cgGLSetParameter3f( m_cgp_IG_f3GridWidthHeightDepth,
		m_iGridWidth, m_iGridHeight, m_iGridDepth );

	glClear( GL_COLOR_BUFFER_BIT );
	GLUtilities::setupOrthoCamera( m_iGridTextureWidth, m_iGridTextureHeight );
	m_pInitializeGridProgram->bind();
	GLUtilities::drawQuad( m_iGridTextureWidth, m_iGridTextureHeight );
}

void InteractiveToneMapKernel::sliceGridAndToneMapPass()
{
	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pOutputTexture );

	// setup input
	cgGLSetTextureParameter( m_cgp_SGTM_inputSampler,
		m_pInputTexture->getTextureId() );
	cgGLSetTextureParameter( m_cgp_SGTM_logBaseSampler,
		m_pBaseTexture->getTextureId() );
	m_pGridTexture->setFilterMode( GLTexture::FILTER_MODE_LINEAR, GLTexture::FILTER_MODE_LINEAR );
	cgGLSetTextureParameter( m_cgp_SGTM_baseGridSampler,
		m_pGridTexture->getTextureId() );

	// setup uniforms
	cgGLSetParameter3f( m_cgp_SGTM_f3_rcpSigma,
		1.f / m_fSigmaSpatial, 1.f / m_fSigmaSpatial, 1.f / m_fSigmaRange );
	cgGLSetParameter3f( m_cgp_SGTM_f3_gridSize,
		m_iGridWidth, m_iGridHeight, m_iGridDepth );
	cgGLSetParameter2f( m_cgp_SGTM_f2LogBaseMinMax,
		m_fLogBaseMin, m_fLogBaseMax );
	
	// TODO: doesn't matter
	cgGLSetParameter1f( m_cgp_SGTM_fLogAbsoluteScale,
		m_fLogBaseMax * m_fCompressionFactor );

	// render
	glClear( GL_COLOR_BUFFER_BIT );
	GLUtilities::setupOrthoCamera( m_iInputWidth, m_iInputHeight );
	m_pSliceGridAndToneMapProgram->bind();
	GLUtilities::drawQuad( m_iInputWidth, m_iInputHeight );

	m_pGridTexture->setFilterMode( GLTexture::FILTER_MODE_NEAREST, GLTexture::FILTER_MODE_NEAREST );
}

void InteractiveToneMapKernel::getGridXYFromClick( int x, int y, int* pGridXOut, int* pGridYOut )
{
	*pGridXOut = Arithmetic::roundToInt( x / m_fSigmaSpatial + m_iPaddingXY );
	*pGridYOut = Arithmetic::roundToInt( y / m_fSigmaSpatial + m_iPaddingXY );	
}

void InteractiveToneMapKernel::getGridXYZFromClick( int x, int y, int* pGridXOut, int* pGridYOut, float* pGridZOut )
{
	// read rgb( x, y )
	float baseValue;

	// try reading input
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pBaseTexture );
	glReadPixels( x, y, 1, 1, GL_RED, GL_FLOAT, &baseValue );
	m_pFBO->detachTexture( GL_COLOR_ATTACHMENT0_EXT );

	printf( "log( baseValue ) = %f, baseValue = %f\n", baseValue, pow( 10.f, baseValue ) - ColorUtils::LOG_LUMINANCE_EPSILON );

	// map base value to [ 0, 1 ]
	float baseValueZeroOne = ( baseValue - m_fLogBaseMin ) / ( m_fLogBaseMax - m_fLogBaseMin );

	getGridXYFromClick( x, y, pGridXOut, pGridYOut );
	*pGridZOut = baseValueZeroOne / m_fSigmaRange + m_iPaddingZ;
}

void InteractiveToneMapKernel::readGridAtTwoZs( int gridX, int gridY, int gridZ0, int gridZ1, float* pGridZ0ValueOut, float* pGridZ1ValueOut )
{
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pGridTexture );
	glReadPixels( gridZ0 * m_iGridWidth + gridX, gridY, 1, 1, GL_RED, GL_FLOAT, pGridZ0ValueOut );
	glReadPixels( gridZ1 * m_iGridWidth + gridX, gridY, 1, 1, GL_RED, GL_FLOAT, pGridZ1ValueOut );
	m_pFBO->detachTexture( GL_COLOR_ATTACHMENT0_EXT );
}
