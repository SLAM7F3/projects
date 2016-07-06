#include "BilateralKernel.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <Cg/CgProgramManager.h>
#include <Cg/CgProgramWrapper.h>
#include <Cg/CgShared.h>
#include <Cg/CgUtilities.h>
#include <GL/GLFramebufferObject.h>
#include <GL/GLShared.h>
#include <GL/GLTextureRectangle.h>
#include <GL/GLUtilities.h>
#include <GL/GLVertexBufferObject.h>
#include <math/Arithmetic.h>
#include <math/MathUtils.h>
#include <math/Random.h>
#include <math/Sampling.h>
#include <math/SamplingPatternND.h>
#include <time/StopWatch.h>

#include "AppData.h"

//////////////////////////////////////////////////////////////////////////
// Public
//////////////////////////////////////////////////////////////////////////

// static
void BilateralKernel::initializeCg()
{
	QString cgPrefix = AppData::getInstance()->getCgPathPrefix();
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	CGprofile latestCgVertexProfile = CgShared::getInstance()->getLatestVertexProfile();
	CGprofile latestCgFragmentProfile = CgShared::getInstance()->getLatestFragmentProfile();

	//////////////////////////////////////////////////////////////////////////

	// ---- Vertex ----
	pProgramManager->loadProgramFromFile( "BF_pointScatterVertex", cgPrefix + "Bilateral.cg",
		"pointScatterVertex", latestCgVertexProfile, NULL );

	// ---- Fragment ----

	pProgramManager->loadProgramFromFile( "BF_pointScatterFragment", cgPrefix + "Bilateral.cg",
		"pointScatterFragment", latestCgFragmentProfile, NULL );

	//////////////////////////////////////////////////////////////////////////

	pProgramManager->loadProgramFromFile( "BF_gaussianBlurLine", cgPrefix + "Bilateral.cg",
		"gaussianBlurLine", latestCgFragmentProfile, NULL );

	//////////////////////////////////////////////////////////////////////////

	pProgramManager->loadProgramFromFile( "BF_gaussianBlurZLineExponentialDecayAndDivide", cgPrefix + "Bilateral.cg",
		"gaussianBlurZLineExponentialDecayAndDivide", latestCgFragmentProfile, NULL );

	//////////////////////////////////////////////////////////////////////////

	pProgramManager->loadProgramFromFile( "BF_slice", cgPrefix + "Bilateral.cg",
		"slice", latestCgFragmentProfile, NULL );

	s_bCgInitialized = true;
}

// static
BilateralKernel* BilateralKernel::create( QString args )
{
	if( !s_bCgInitialized )
	{
		initializeCg();
	}

	BilateralKernel* pInstance = new BilateralKernel( args );
	pInstance->initializeGL();
	return pInstance;
}

// virtual
BilateralKernel::~BilateralKernel()
{
	deleteInternalVBOs();
	deleteOutputTexture();
}

// virtual
void BilateralKernel::makeDirty( QString inputPortName )
{
	if( inputPortName == "sigmaSpatial" ||
		inputPortName == "sigmaRange" ||
		inputPortName == "inputTexture" )
	{
		m_bReallocationNeeded = true;
	}

	m_pBilateralFilterOutputPort->makeDirty();
}

// virtual
void BilateralKernel::compute( QString outputPortName )							 
{
	if( m_bReallocationNeeded )
	{
		reallocate();		
	}

	m_fTemporalDecayLambda = m_pTemporalDecayLambdaInputPort->pullData().getFloatData();

	pointScatterPass();
	blurXPass();
	blurYPass();
	blurZDecayDividePass();

	GLUtilities::setupOrthoCamera( m_iInputWidth, m_iInputHeight ); // no vertex program
	slicePass();
}

//////////////////////////////////////////////////////////////////////////
// protected
//////////////////////////////////////////////////////////////////////////

// virtual
bool BilateralKernel::isInputComplete()
{
	return true;
}

// virtual
void BilateralKernel::initializeGL()
{
	GPUKernel::initializeGL();
	initializeCgPrograms();	

	int maxTextureSize = GLTexture::getMaxTextureSize();

	// for video
	// TODO: make this an arg
	m_iInternalTextureWidth = maxTextureSize;
// 	m_iInternalTextureWidth = 2048;
	int nBits = 32;
	m_iInternalTextureHeight = 256;

	// TODO: shared on nComponents and nBits, maybe format?

	// can't grab 2 shared scratch space - we have to keep 1 around for the next pass
	QVector< GLTextureRectangle* > tempGrids =
		GLShared::getInstance()->getSharedTexture
		(
			m_iInternalTextureWidth, m_iInternalTextureHeight, 1
		);
	m_pTempGrid0 = tempGrids[ 0 ];	
		
	m_pTempGrid1 = GLTextureRectangle::createFloat4Texture( m_iInternalTextureWidth, m_iInternalTextureHeight, nBits );
	m_pPreviousGrid = GLTextureRectangle::createFloat4Texture( m_iInternalTextureWidth, m_iInternalTextureHeight, nBits );
}

// virtual
void BilateralKernel::initializePorts()
{
	// ---- Input Ports ----	

	// float array
	m_pSigmaSpatialInputPort = addInputPort( "sigmaSpatial", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pSigmaSpatialInputPort->setFloatMinMaxDelta( 16.f, 5.f, 128.f, 1.f );

	m_pSigmaRangeInputPort = addInputPort( "sigmaRange", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pSigmaRangeInputPort->setFloatMinMaxDelta( 10.f, 1.f, 100.f, 1.f );

	// temporal decay
	m_pTemporalDecayLambdaInputPort = addInputPort( "temporalDecayLambda", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pTemporalDecayLambdaInputPort->setFloatMinMaxDelta( 0.f, 0.f, 20.f, 1.f );	

	// input texture
	m_pInputTextureInputPort = addInputPort( "inputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
	m_pInputMinInputPort = addInputPort( "inputMin", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pInputMaxInputPort = addInputPort( "inputMax", KERNEL_PORT_DATA_TYPE_FLOAT );

	// ---- Output Ports ----
	m_pBilateralFilterOutputPort = addOutputPort( "outputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
}

void BilateralKernel::pointScatterPass()
{
	// setup output
	cgGLEnableProfile( CgShared::getInstance()->getLatestVertexProfile() );
	glEnable( GL_BLEND );

	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pTempGrid0 );

	// setup inputs
	cgGLSetTextureParameter( m_cgp_PSV_inputSampler,
		m_pInputTexture->getTextureId() );
	
	// setup uniforms
	CgUtilities::setupOrthoCamera( m_iInternalTextureWidth, m_iInternalTextureHeight, m_cgp_PSV_f44_mvp );
	cgGLSetParameter3f( m_cgp_PSV_f3_rcpSigma,
		1.f / m_fSigmaSpatial, 1.f / m_fSigmaSpatial, 1.f / m_fSigmaRange );
	cgGLSetParameter3f( m_cgp_PSV_f3_gridSize, m_iGridWidth, m_iGridHeight, m_iGridDepth );
	cgGLSetParameter1f( m_cgp_PSV_f_inputMin, m_fInputMin );
	
	// render
	glClear( GL_COLOR_BUFFER_BIT );		
	m_pPointScatterVertexProgram->bind();
	m_pPointScatterFragmentProgram->bind();

	m_pInputXYCoordinateVBO->bind( GLBufferObject::TARGET_ARRAY_BUFFER );
	glVertexPointer( 2, GL_SHORT, 0, NULL );

	if( m_bUseSubsampling )
	{
		glDrawArrays( GL_POINTS, m_iCurrentPattern * m_nPixelsPerPattern, m_nPixelsPerPattern );
		m_iCurrentPattern = ( m_iCurrentPattern + 1 ) % m_nPatterns;
	}
	else
	{
		glDrawArrays( GL_POINTS, 0, m_pInputXYCoordinateVBO->getNumElements() );
	}

	// restore state
	GLBufferObject::unbind( GLBufferObject::TARGET_ARRAY_BUFFER );
	glDisable( GL_BLEND );
	cgGLDisableProfile( CgShared::getInstance()->getLatestVertexProfile() );
}

void BilateralKernel::blurXPass()
{
	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pTempGrid1 );

	// setup input
	cgGLSetTextureParameter( m_cgp_GBL_gridSampler,
		m_pTempGrid0->getTextureId() );

	// setup uniforms
	cgGLSetParameter2f( m_cgp_GBL_f2Delta, 1, 0 );
	cgGLSetParameter2f( m_cgp_GBL_f2TwoDelta, 2, 0 );

	glClear( GL_COLOR_BUFFER_BIT );
	m_pGaussianBlurLineProgram->bind();
	GLUtilities::drawQuad( m_iGridTextureWidth, m_iGridTextureHeight ); // draw only the grid region
}

void BilateralKernel::blurYPass()
{
	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pTempGrid0 );

	// setup input
	cgGLSetTextureParameter( m_cgp_GBL_gridSampler,
		m_pTempGrid1->getTextureId() );

	// setup uniforms
	cgGLSetParameter2f( m_cgp_GBL_f2Delta, 0, 1 );
	cgGLSetParameter2f( m_cgp_GBL_f2TwoDelta, 0, 2 );

	glClear( GL_COLOR_BUFFER_BIT );
	m_pGaussianBlurLineProgram->bind();
	GLUtilities::drawQuad( m_iGridTextureWidth, m_iGridTextureHeight ); // draw only the grid region
}

void BilateralKernel::blurZDecayDividePass()
{
	float normalization = 1.f / ( 1.f + m_fTemporalDecayLambda );

	// setup output: render into tempGrid1
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pTempGrid1 );

	// setup input - blurYPass() rendered into tempGrid0
	cgGLSetTextureParameter( m_cgp_GBZLEDD_gridSampler,
		m_pTempGrid0->getTextureId() );	
	cgGLSetTextureParameter( m_cgp_GBZLEDD_previousGridSampler,
		m_pPreviousGrid->getTextureId() );
	
	// set uniforms
	cgGLSetParameter2f( m_cgp_GBZLEDD_f2Delta, m_iGridWidth, 0 );
	cgGLSetParameter2f( m_cgp_GBZLEDD_f2TwoDelta, 2 * m_iGridWidth, 0 );
	cgGLSetParameter2f( m_cgp_GBZLEDD_f2TemporalDecayLambdaNormalization,
		m_fTemporalDecayLambda, normalization );

	glClear( GL_COLOR_BUFFER_BIT );
	m_pGaussianBlurZLineExponentialDecayAndDivideProgram->bind();
	GLUtilities::drawQuad( m_iGridTextureWidth, m_iGridTextureHeight ); // draw only the grid region

	// swap tempGrid1 with previousGrid
	GLTextureRectangle* pTemp = m_pPreviousGrid;
	m_pPreviousGrid = m_pTempGrid1;
	m_pTempGrid1 = pTemp;
}

void BilateralKernel::slicePass()
{
	// retrieve parameters
	GLTextureRectangle* pQuotientGrid = m_pPreviousGrid; // it was swapped in blurZDecayDividePass()
	
	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pOutputTexture );

	// setup inputs
	cgGLSetTextureParameter( m_cgp_S_inputSampler,
		m_pInputTexture->getTextureId() );
	pQuotientGrid->setFilterMode( GLTexture::FILTER_MODE_LINEAR, GLTexture::FILTER_MODE_LINEAR );
	cgGLSetTextureParameter( m_cgp_S_quotientGridSampler,
		pQuotientGrid->getTextureId() );

	// set uniforms
	cgGLSetParameter3f( m_cgp_S_f3_rcpSigma,
		1.f / m_fSigmaSpatial, 1.f / m_fSigmaSpatial, 1.f / m_fSigmaRange );
	cgGLSetParameter3f( m_cgp_S_f3_gridSize, m_iGridWidth, m_iGridHeight, m_iGridDepth );
	cgGLSetParameter1f( m_cgp_S_f_inputMin, m_fInputMin );

	glClear( GL_COLOR_BUFFER_BIT );
	m_pSliceProgram->bind();
	GLUtilities::drawQuad( m_iInputWidth, m_iInputHeight );

	pQuotientGrid->setFilterMode( GLTexture::FILTER_MODE_NEAREST , GLTexture::FILTER_MODE_NEAREST );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

// static
bool BilateralKernel::s_bCgInitialized = false;

BilateralKernel::BilateralKernel( QString args ) :

	GPUKernel( "Bilateral" ),

	m_pFBO( GLShared::getInstance()->getSharedFramebufferObject() ),

	m_iPadding( 3 ),	

	m_iInputWidth( -1 ),
	m_iInputHeight( -1 ),
	m_nOutputs( -1 ),
	m_bReallocationNeeded( true ),

	m_pInputXYCoordinateVBO( NULL ),
	m_pTempGrid0( NULL ),
	m_pTempGrid1( NULL ),
	m_pOutputTexture( NULL ),

	m_nPatterns( 5 )	
{
	if( args == "subsample" )
	{
		m_bUseSubsampling = true;
	}
	else
	{
		m_bUseSubsampling = false;
	}
}

void BilateralKernel::deleteInternalVBOs()
{
	if( m_pInputXYCoordinateVBO != NULL )
	{
		delete m_pInputXYCoordinateVBO;
		m_pInputXYCoordinateVBO = NULL;
	}
}

void BilateralKernel::deleteOutputTexture()
{
	if( m_pOutputTexture != NULL )
	{
		delete m_pOutputTexture;
		m_pOutputTexture = NULL;
	}
}

void BilateralKernel::reallocate()
{
	// read input texture
	m_pInputTexture = m_pInputTextureInputPort->pullData().getGLTextureRectangleData();	

	// figure out if the input size has changed
	bool sizeChanged = false;
	if( m_pInputTexture->getWidth() != m_iInputWidth )
	{
		m_iInputWidth = m_pInputTexture->getWidth();
		sizeChanged = true;
	}
	if( m_pInputTexture->getHeight() != m_iInputHeight )
	{
		m_iInputHeight = m_pInputTexture->getHeight();
		sizeChanged = true;
	}
	m_nInputPixels = m_iInputWidth * m_iInputHeight;
	
	if( sizeChanged )
	{
		reallocateOutputTexture();
		reallocateInternalVBOs();
	}

	// read new parameters
	m_fSigmaSpatial = m_pSigmaSpatialInputPort->pullData().getFloatData();
	m_fSigmaRange = m_pSigmaRangeInputPort->pullData().getFloatData();
	m_fInputMin = m_pInputMinInputPort->pullData().getFloatData();
	m_fInputMax = m_pInputMaxInputPort->pullData().getFloatData();
	m_fInputDelta = m_fInputMax - m_fInputMin;

	m_iGridWidth = static_cast< int >( ( m_iInputWidth - 1 ) / m_fSigmaSpatial ) + 1 + 2 * m_iPadding;
	m_iGridHeight = static_cast< int >( ( m_iInputHeight - 1 ) / m_fSigmaSpatial ) + 1 + 2 * m_iPadding;
	m_iGridDepth = static_cast< int >( m_fInputDelta / m_fSigmaRange ) + 1 + 2 * m_iPadding;

	m_iGridTextureWidth = m_iGridDepth * m_iGridWidth;
	m_iGridTextureHeight = m_iGridHeight;		

	// assert( m_iGridTextureWidth <= m_iInternalTextureWidth );
	// assert( m_iGridTextureHeight <= m_iInternalTextureHeight );

	m_bReallocationNeeded = false;
}

void BilateralKernel::initializeCgPrograms()
{
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();

	/////////////////////////////////////////////////////////////////////	

	m_pPointScatterVertexProgram = pProgramManager->getNamedProgram( "BF_pointScatterVertex" );
	
	m_cgp_PSV_f44_mvp = m_pPointScatterVertexProgram->getNamedParameter( "mvp" );
	m_cgp_PSV_f3_rcpSigma = m_pPointScatterVertexProgram->getNamedParameter( "rcpSigma" );
	m_cgp_PSV_f3_gridSize = m_pPointScatterVertexProgram->getNamedParameter( "gridSize" );
	m_cgp_PSV_f_inputMin = m_pPointScatterVertexProgram->getNamedParameter( "inputMin" );
	m_cgp_PSV_inputSampler = m_pPointScatterVertexProgram->getNamedParameter( "inputSampler" );

	/////////////////////////////////////////////////////////////////////

	m_pPointScatterFragmentProgram = pProgramManager->getNamedProgram( "BF_pointScatterFragment" );	

	/////////////////////////////////////////////////////////////////////	

	m_pGaussianBlurLineProgram = pProgramManager->getNamedProgram( "BF_gaussianBlurLine" );	
	m_cgp_GBL_gridSampler = m_pGaussianBlurLineProgram->getNamedParameter( "gridSampler" );
	m_cgp_GBL_f2Delta = m_pGaussianBlurLineProgram->getNamedParameter( "delta" );
	m_cgp_GBL_f2TwoDelta = m_pGaussianBlurLineProgram->getNamedParameter( "twoDelta" );

	/////////////////////////////////////////////////////////////////////	

	m_pGaussianBlurZLineExponentialDecayAndDivideProgram = pProgramManager->getNamedProgram( "BF_gaussianBlurZLineExponentialDecayAndDivide" );
	m_cgp_GBZLEDD_gridSampler = m_pGaussianBlurZLineExponentialDecayAndDivideProgram->getNamedParameter( "gridSampler" );
	m_cgp_GBZLEDD_previousGridSampler = m_pGaussianBlurZLineExponentialDecayAndDivideProgram->getNamedParameter( "previousGridSampler" );
	m_cgp_GBZLEDD_f2Delta = m_pGaussianBlurZLineExponentialDecayAndDivideProgram->getNamedParameter( "delta" );
	m_cgp_GBZLEDD_f2TwoDelta = m_pGaussianBlurZLineExponentialDecayAndDivideProgram->getNamedParameter( "twoDelta" );
	m_cgp_GBZLEDD_f2TemporalDecayLambdaNormalization = m_pGaussianBlurZLineExponentialDecayAndDivideProgram->getNamedParameter( "temporalDecayLambdaNormalization" );

	/////////////////////////////////////////////////////////////////////	

	m_pSliceProgram = pProgramManager->getNamedProgram( "BF_slice" );
	m_cgp_S_inputSampler = m_pSliceProgram->getNamedParameter( "inputSampler" );
	m_cgp_S_quotientGridSampler = m_pSliceProgram->getNamedParameter( "quotientGridSampler" );
	m_cgp_S_f3_rcpSigma = m_pSliceProgram->getNamedParameter( "rcpSigma" );
	m_cgp_S_f3_gridSize = m_pSliceProgram->getNamedParameter( "gridSize" );
	m_cgp_S_f_inputMin = m_pSliceProgram->getNamedParameter( "inputMin" );
}

void BilateralKernel::reallocateOutputTexture()
{
	deleteOutputTexture();

	int nBits = AppData::getInstance()->getTextureNumBits();
	m_pOutputTexture = GLTextureRectangle::createFloat4Texture( m_iInputWidth, m_iInputHeight, nBits );

	// set new data on output port
	m_pBilateralFilterOutputPort->pushData( KernelPortData( m_pOutputTexture ) );
}

void BilateralKernel::reallocateInternalVBOs()
{
	deleteInternalVBOs();

	if( m_bUseSubsampling )
	{
		Random random;
		m_nPixelsPerPattern = Arithmetic::roundToInt( 0.1 * m_nInputPixels );		

		// concatenate all the patterns together
		GLshort* xyCoords = new GLshort[ 2 * m_nPixelsPerPattern * m_nPatterns ];

		for( int i = 0; i < m_nPatterns; ++i )
		{
			SamplingPatternND samplingPattern( m_nPixelsPerPattern, 2 );
			Sampling::latinHypercubeSampling( random, &samplingPattern );

			for( int j = 0; j < m_nPixelsPerPattern; ++j )
			{
				float sample[2];
				samplingPattern.getSample( j, sample );

				xyCoords[ 2 * ( i * m_nPixelsPerPattern + j ) ] = Arithmetic::roundToInt( m_iInputWidth * sample[ 0 ] );
				xyCoords[ 2 * ( i * m_nPixelsPerPattern + j ) + 1 ] = Arithmetic::roundToInt( m_iInputHeight * sample[ 1 ] );
			}
		}

		m_pInputXYCoordinateVBO = GLVertexBufferObject::fromShortArray
			(
				xyCoords, 2 * m_nPixelsPerPattern * m_nPatterns, m_nPixelsPerPattern * m_nPatterns
			);

		delete[] xyCoords;

		m_iCurrentPattern = 0;
	}
	else
	{
		m_pInputXYCoordinateVBO = GLShared::getInstance()->getSharedXYCoordinateVBO
			(
				m_iInputWidth, m_iInputHeight
			);
	}	
}
