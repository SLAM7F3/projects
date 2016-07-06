#include "CrossBilateralKernel.h"

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
#include <time/StopWatch.h>

#include "AppData.h"

//////////////////////////////////////////////////////////////////////////
// Public
//////////////////////////////////////////////////////////////////////////

// static
void CrossBilateralKernel::initializeCg()
{
	QString cgPrefix = AppData::getInstance()->getCgPathPrefix();
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	CGprofile latestCgVertexProfile = CgShared::getInstance()->getLatestVertexProfile();
	CGprofile latestCgFragmentProfile = CgShared::getInstance()->getLatestFragmentProfile();

	//////////////////////////////////////////////////////////////////////////

	// ---- Vertex ----
	
	pProgramManager->loadProgramFromFile( "CBF_pointScatterVertex", cgPrefix + "CrossBilateral.cg",
		"pointScatterVertex", latestCgVertexProfile, NULL );

	// ---- Fragment ----

	pProgramManager->loadProgramFromFile( "CBF_pointScatterFragment", cgPrefix + "CrossBilateral.cg",
		"pointScatterFragment", latestCgFragmentProfile, NULL );

	//////////////////////////////////////////////////////////////////////////

	pProgramManager->loadProgramFromFile( "CBF_gaussianBlurLine", cgPrefix + "CrossBilateral.cg",
		"gaussianBlurLine", latestCgFragmentProfile, NULL );

	//////////////////////////////////////////////////////////////////////////

	pProgramManager->loadProgramFromFile( "CBF_gaussianBlurZLineAndDivide", cgPrefix + "CrossBilateral.cg",
		"gaussianBlurZLineAndDivide", latestCgFragmentProfile, NULL );

	//////////////////////////////////////////////////////////////////////////

	pProgramManager->loadProgramFromFile( "CBF_slice", cgPrefix + "CrossBilateral.cg",
		"slice", latestCgFragmentProfile, NULL );

	s_bCgInitialized = true;
}

// static
CrossBilateralKernel* CrossBilateralKernel::create( QString args )
{
	if( !s_bCgInitialized )
	{
		initializeCg();
	}

	CrossBilateralKernel* pInstance = new CrossBilateralKernel( args );
	pInstance->initializeGL();
	return pInstance;
}

// virtual
CrossBilateralKernel::~CrossBilateralKernel()
{
	deleteInternalVBOs();
	deleteOutputTexture();
}

// virtual
void CrossBilateralKernel::makeDirty( QString inputPortName )
{
	if( inputPortName == "sigmaSpatial" ||
		inputPortName == "sigmaRange" ||
		inputPortName == "edgeTexture" ||
		inputPortName == "dataTexture" )
	{
		m_bReallocationNeeded = true;
	}

	m_pOutputPort->makeDirty();
}

// virtual
void CrossBilateralKernel::compute( QString outputPortName )							 
{
	if( m_bReallocationNeeded )
	{
		reallocate();		
	}

	pointScatterPass();
	blurXPass();
	blurYPass();
	blurZAndDividePass();

	GLUtilities::setupOrthoCamera( m_iInputWidth, m_iInputHeight ); // no vertex program
	slicePass();
}

//////////////////////////////////////////////////////////////////////////
// protected
//////////////////////////////////////////////////////////////////////////

// virtual
bool CrossBilateralKernel::isInputComplete()
{
	return true;
}

// virtual
void CrossBilateralKernel::initializeGL()
{
	GPUKernel::initializeGL();
	initializeCgPrograms();	

	int maxTextureSize = GLTexture::getMaxTextureSize();

	// for video
	// m_iInternalTextureWidth = maxTextureSize;
	// TODO: make this an arg
	m_iInternalTextureWidth = 2048;
	m_iInternalTextureHeight = 256;

	// TODO: shared on nComponents and nBits, maybe format?
	QVector< GLTextureRectangle* > tempGrids =
		GLShared::getInstance()->getSharedTexture
		(
			m_iInternalTextureWidth, m_iInternalTextureHeight, 2
		);

	m_pTempGrid0 = tempGrids[ 0 ];
	m_pTempGrid1 = tempGrids[ 1 ];
}

// virtual
void CrossBilateralKernel::initializePorts()
{
	// ---- Input Ports ----

	// floats
	m_pSigmaSpatialInputPort = addInputPort( "sigmaSpatial", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pSigmaSpatialInputPort->setFloatMinMaxDelta( 16.f, 5.f, 128.f, 1.f );

	m_pSigmaRangeInputPort = addInputPort( "sigmaRange", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pSigmaRangeInputPort->setFloatMinMaxDelta( 10.f, 1.f, 100.f, 1.f );

	m_pEdgeMinInputPort = addInputPort( "edgeMin", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pEdgeMaxInputPort = addInputPort( "edgeMax", KERNEL_PORT_DATA_TYPE_FLOAT );

	// textures
	m_pEdgeTextureInputPort = addInputPort( "edgeTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
	m_pDataTextureInputPort = addInputPort( "dataTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );

	// ---- Output Ports ----
	m_pOutputPort = addOutputPort( "outputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
}

void CrossBilateralKernel::pointScatterPass()
{
	// setup output
	cgGLEnableProfile( CgShared::getInstance()->getLatestVertexProfile() );
	glEnable( GL_BLEND );
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pTempGrid0 );

	// setup inputs
	cgGLSetTextureParameter( m_cgp_PSV_edgeSampler,
		m_pInputEdgeTexture->getTextureId() );
	cgGLSetTextureParameter( m_cgp_PSV_dataSampler,
		m_pInputDataTexture->getTextureId() );

	// setup uniforms
	CgUtilities::setupOrthoCamera( m_iInternalTextureWidth, m_iInternalTextureHeight, m_cgp_PSV_f44_mvp );
	cgGLSetParameter3f( m_cgp_PSV_f3_rcpSigma,
		1.f / m_fSigmaSpatial, 1.f / m_fSigmaSpatial, 1.f / m_fSigmaRange );
	cgGLSetParameter3f( m_cgp_PSV_f3_gridSize,
		m_iGridWidth, m_iGridHeight, m_iGridDepth );
	cgGLSetParameter1f( m_cgp_PSV_f_edgeMin, m_fEdgeMin );

	// render
	glClear( GL_COLOR_BUFFER_BIT );
	m_pPointScatterVertexProgram->bind();
	m_pPointScatterFragmentProgram->bind();

	m_pInputXYCoordinateVBO->bind( GLBufferObject::TARGET_ARRAY_BUFFER );
	glVertexPointer( 2, GL_SHORT, 0, NULL );
	glDrawArrays( GL_POINTS, 0, m_pInputXYCoordinateVBO->getNumElements() );

	// restore state
	GLBufferObject::unbind( GLBufferObject::TARGET_ARRAY_BUFFER );
	glDisable( GL_BLEND );
	cgGLDisableProfile( CgShared::getInstance()->getLatestVertexProfile() );
}

void CrossBilateralKernel::blurXPass()
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

void CrossBilateralKernel::blurYPass()
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

void CrossBilateralKernel::blurZAndDividePass()
{
	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pTempGrid1 );

	// setup input
	cgGLSetTextureParameter( m_cgp_GBZLDD_gridSampler,
		m_pTempGrid0->getTextureId() );

	// set uniforms
	cgGLSetParameter2f( m_cgp_GBZLDD_f2Delta, m_iGridWidth, 0 );
	cgGLSetParameter2f( m_cgp_GBZLDD_f2TwoDelta, 2 * m_iGridWidth, 0 );

	glClear( GL_COLOR_BUFFER_BIT );
	m_pGaussianBlurZLineAndDivideProgram->bind();
	GLUtilities::drawQuad( m_iGridTextureWidth, m_iGridTextureHeight ); // draw only the grid region
}

void CrossBilateralKernel::slicePass()
{
	GLTextureRectangle* pQuotientGrid = m_pTempGrid1;
	
	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pOutputTexture );

	// setup inputs
	cgGLSetTextureParameter( m_cgp_S_edgeSampler,
		m_pInputEdgeTexture->getTextureId() );
	pQuotientGrid->setFilterMode( GLTexture::FILTER_MODE_LINEAR, GLTexture::FILTER_MODE_LINEAR );
	cgGLSetTextureParameter( m_cgp_S_quotientGridSampler,
		pQuotientGrid->getTextureId() );

	// set uniforms
	cgGLSetParameter3f( m_cgp_S_f3_rcpSigma,
		1.f / m_fSigmaSpatial, 1.f / m_fSigmaSpatial, 1.f / m_fSigmaRange );
	cgGLSetParameter3f( m_cgp_S_f3_gridSize, m_iGridWidth, m_iGridHeight, m_iGridDepth );
	cgGLSetParameter1f( m_cgp_S_f_edgeMin, m_fEdgeMin );

	glClear( GL_COLOR_BUFFER_BIT );
	m_pSliceProgram->bind();
	GLUtilities::drawQuad( m_iInputWidth, m_iInputHeight );

	pQuotientGrid->setFilterMode( GLTexture::FILTER_MODE_NEAREST , GLTexture::FILTER_MODE_NEAREST );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

// static
bool CrossBilateralKernel::s_bCgInitialized = false;

CrossBilateralKernel::CrossBilateralKernel( QString args ) :

	GPUKernel( "CrossBilateral" ),

	m_pFBO( GLShared::getInstance()->getSharedFramebufferObject() ),

	m_iPadding( 3 ),	

	m_iInputWidth( -1 ),
	m_iInputHeight( -1 ),
	m_bReallocationNeeded( true ),

	m_pInputXYCoordinateVBO( NULL ),
	m_pTempGrid0( NULL ),
	m_pTempGrid1( NULL ),

	m_pOutputTexture( NULL )
{

}

void CrossBilateralKernel::deleteInternalVBOs()
{
	if( m_pInputXYCoordinateVBO != NULL )
	{
		delete m_pInputXYCoordinateVBO;
		m_pInputXYCoordinateVBO = NULL;
	}
}

void CrossBilateralKernel::deleteOutputTexture()
{
	if( m_pOutputTexture != NULL )
	{
		delete m_pOutputTexture;
		m_pOutputTexture = NULL;
	}
}

void CrossBilateralKernel::reallocate()
{
	// read input texture
	m_pInputEdgeTexture = m_pEdgeTextureInputPort->pullData().getGLTextureRectangleData();	
	m_pInputDataTexture = m_pDataTextureInputPort->pullData().getGLTextureRectangleData();

	// figure out if the input size has changed
	bool sizeChanged = false;
	if( m_pInputEdgeTexture->getWidth() != m_iInputWidth )
	{
		m_iInputWidth = m_pInputEdgeTexture->getWidth();
		sizeChanged = true;
	}
	if( m_pInputEdgeTexture->getHeight() != m_iInputHeight )
	{
		m_iInputHeight = m_pInputEdgeTexture->getHeight();
		sizeChanged = true;
	}
	m_nInputPixels = m_iInputWidth * m_iInputHeight;

	assert( m_pInputDataTexture->getWidth() == m_iInputWidth );
	assert( m_pInputDataTexture->getHeight() == m_iInputHeight );	

	if( sizeChanged )
	{
		reallocateOutputTexture();
		reallocateInternalVBOs();
	}

	// read new parameters
	m_fSigmaSpatial = m_pSigmaSpatialInputPort->pullData().getFloatData();
	m_fSigmaRange = m_pSigmaRangeInputPort->pullData().getFloatData();
	m_fEdgeMin = m_pEdgeMinInputPort->pullData().getFloatData();
	m_fEdgeMax = m_pEdgeMaxInputPort->pullData().getFloatData();
	m_fEdgeDelta = m_fEdgeMax - m_fEdgeMin;

	m_iGridWidth = static_cast< int >( ( m_iInputWidth - 1 ) / m_fSigmaSpatial ) + 1 + 2 * m_iPadding;
	m_iGridHeight = static_cast< int >( ( m_iInputHeight - 1 ) / m_fSigmaSpatial ) + 1 + 2 * m_iPadding;
	m_iGridDepth = static_cast< int >( m_fEdgeDelta / m_fSigmaRange ) + 1 + 2 * m_iPadding;

	m_iGridTextureWidth = m_iGridDepth * m_iGridWidth;
	m_iGridTextureHeight = m_iGridHeight;

	// assert( m_iGridTextureWidth <= m_iInternalTextureWidth );
	// assert( m_iGridTextureHeight <= m_iInternalTextureHeight );

	m_bReallocationNeeded = false;
}

void CrossBilateralKernel::initializeCgPrograms()
{
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();

	/////////////////////////////////////////////////////////////////////

	m_pPointScatterVertexProgram = pProgramManager->getNamedProgram( "CBF_pointScatterVertex" );

	m_cgp_PSV_f44_mvp = m_pPointScatterVertexProgram->getNamedParameter( "mvp" );
	m_cgp_PSV_f3_rcpSigma = m_pPointScatterVertexProgram->getNamedParameter( "rcpSigma" );
	m_cgp_PSV_f3_gridSize = m_pPointScatterVertexProgram->getNamedParameter( "gridSize" );
	m_cgp_PSV_f_edgeMin = m_pPointScatterVertexProgram->getNamedParameter( "edgeMin" );
	m_cgp_PSV_edgeSampler = m_pPointScatterVertexProgram->getNamedParameter( "edgeSampler" );
	m_cgp_PSV_dataSampler = m_pPointScatterVertexProgram->getNamedParameter( "dataSampler" );

	/////////////////////////////////////////////////////////////////////	

	m_pPointScatterFragmentProgram = pProgramManager->getNamedProgram( "CBF_pointScatterFragment" );	

	/////////////////////////////////////////////////////////////////////	

	m_pGaussianBlurLineProgram = pProgramManager->getNamedProgram( "CBF_gaussianBlurLine" );	
	m_cgp_GBL_gridSampler = m_pGaussianBlurLineProgram->getNamedParameter( "gridSampler" );
	m_cgp_GBL_f2Delta = m_pGaussianBlurLineProgram->getNamedParameter( "delta" );
	m_cgp_GBL_f2TwoDelta = m_pGaussianBlurLineProgram->getNamedParameter( "twoDelta" );

	/////////////////////////////////////////////////////////////////////	

	m_pGaussianBlurZLineAndDivideProgram = pProgramManager->getNamedProgram( "CBF_gaussianBlurZLineAndDivide" );
	m_cgp_GBZLDD_gridSampler = m_pGaussianBlurZLineAndDivideProgram->getNamedParameter( "gridSampler" );
	m_cgp_GBZLDD_f2Delta = m_pGaussianBlurZLineAndDivideProgram->getNamedParameter( "delta" );
	m_cgp_GBZLDD_f2TwoDelta = m_pGaussianBlurZLineAndDivideProgram->getNamedParameter( "twoDelta" );

	/////////////////////////////////////////////////////////////////////	

	m_pSliceProgram = pProgramManager->getNamedProgram( "CBF_slice" );
	m_cgp_S_edgeSampler = m_pSliceProgram->getNamedParameter( "edgeSampler" );
	m_cgp_S_quotientGridSampler = m_pSliceProgram->getNamedParameter( "quotientGridSampler" );
	m_cgp_S_f3_rcpSigma = m_pSliceProgram->getNamedParameter( "rcpSigma" );
	m_cgp_S_f3_gridSize = m_pSliceProgram->getNamedParameter( "gridSize" );
	m_cgp_S_f_edgeMin = m_pSliceProgram->getNamedParameter( "edgeMin" );
}

void CrossBilateralKernel::reallocateOutputTexture()
{
	deleteOutputTexture();

	int nBits = AppData::getInstance()->getTextureNumBits();
	m_pOutputTexture = GLTextureRectangle::createFloat4Texture( m_iInputWidth, m_iInputHeight, nBits );

	// set new data on output port
	m_pOutputPort->pushData( KernelPortData( m_pOutputTexture ) );
}

void CrossBilateralKernel::reallocateInternalVBOs()
{
	deleteInternalVBOs();

	m_pInputXYCoordinateVBO = GLShared::getInstance()->getSharedXYCoordinateVBO
	(
		m_iInputWidth, m_iInputHeight
	);
}
