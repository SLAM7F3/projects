#include "DifferenceOfGaussiansEdgesKernel.h"

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

#include "AppData.h"

//////////////////////////////////////////////////////////////////////////
// Public
//////////////////////////////////////////////////////////////////////////

// static
void DifferenceOfGaussiansEdgesKernel::initializeCg()
{
	QString cgPrefix = AppData::getInstance()->getCgPathPrefix();
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	CGprofile latestCgFragmentProfile = CgShared::getInstance()->getLatestFragmentProfile();

	pProgramManager->loadProgramFromFile( "DoG_gaussianBlur", cgPrefix + "DoGEdges.cg",
		"gaussianBlur", latestCgFragmentProfile, NULL );

	pProgramManager->loadProgramFromFile( "DoG_gaussianBlurSubtractAndSmoothStep", cgPrefix + "DoGEdges.cg",
		"gaussianBlurSubtractAndSmoothStep", latestCgFragmentProfile, NULL );

	s_bCgInitialized = true;
}

// static
DifferenceOfGaussiansEdgesKernel* DifferenceOfGaussiansEdgesKernel::create( QString args )
{
	if( !s_bCgInitialized )
	{
		initializeCg();
	}

	DifferenceOfGaussiansEdgesKernel* pInstance = new DifferenceOfGaussiansEdgesKernel;
	pInstance->initializeGL();
	return pInstance;
}

// virtual
DifferenceOfGaussiansEdgesKernel::~DifferenceOfGaussiansEdgesKernel()
{
	deleteTextures();
}

// virtual
bool DifferenceOfGaussiansEdgesKernel::isInputComplete()
{
	return true;
}

// virtual
void DifferenceOfGaussiansEdgesKernel::makeDirty( QString inputPortName )
{
	if( inputPortName == "inputTexture" )
	{
		m_bReallocationNeeded = true;
	}

	m_pOutputTextureOutputPort->makeDirty();
}

// virtual
void DifferenceOfGaussiansEdgesKernel::compute( QString outputPortName )
{
	if( m_bReallocationNeeded )
	{
		reallocate();
	}

	// retrieve inputs
	// sigmaE blur passes - share the same sigma
	m_fSigmaE = m_pSigmaEInputPort->pullData().getFloatData();
	m_fTau = m_pTauInputPort->pullData().getFloatData();
	m_fPhi = m_pPhiInputPort->pullData().getFloatData();	
	
	GLUtilities::setupOrthoCamera( m_iInputWidth, m_iInputHeight );

	// do blur( sigmaE )
	blurPass( m_fSigmaE, m_pInputTexture, m_pInternalTempTexture0, 1, 0 ); // read input, write to temp0
	blurPass( m_fSigmaE, m_pInternalTempTexture0, m_pInternalTempTexture1, 0, 1 ); // read temp0, write to temp1

	// do blur( sigmaR )
	const float sigmaFactor = sqrt( 1.6f );
	float sigmaR = sigmaFactor * m_fSigmaE;
	blurPass( sigmaR, m_pInputTexture, m_pInternalTempTexture0, 1, 0 ); // read input, write to temp0
	blurYSubtractAndSmoothStepPass( sigmaR,
		m_pInternalTempTexture0,
		m_pInternalTempTexture1,
		m_pOutputTexture, 0, 1 ); // read temp0 and temp1, write to output
}

//////////////////////////////////////////////////////////////////////////
// Protected
//////////////////////////////////////////////////////////////////////////

// virtual
void DifferenceOfGaussiansEdgesKernel::initializeGL()
{
	GPUKernel::initializeGL();

	initializeCgPrograms();
}

// virtual
void DifferenceOfGaussiansEdgesKernel::initializePorts()
{
	m_pInputTextureInputPort = addInputPort( "inputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );

	m_pSigmaEInputPort = addInputPort( "sigmaE", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pSigmaEInputPort->setFloatMinMaxDelta( 5.f, 1.f, 50.f, 1.f );

	m_pPhiInputPort = addInputPort( "phi", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pPhiInputPort->setFloatMinMaxDelta( 1.f, 0.75f, 5.f, 0.25f );

	m_pTauInputPort = addInputPort( "tau", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pTauInputPort->setFloatMinMaxDelta( 0.1f, -1.f, 1.f, 0.01f );
	
	m_pOutputTextureOutputPort = addOutputPort( "outputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

// static
bool DifferenceOfGaussiansEdgesKernel::s_bCgInitialized = false;

DifferenceOfGaussiansEdgesKernel::DifferenceOfGaussiansEdgesKernel() :

	GPUKernel( "DifferenceOfGaussiansEdges" ),

	m_pFBO( GLShared::getInstance()->getSharedFramebufferObject() ),

	m_bReallocationNeeded( true ),

	m_pInternalTempTexture0( NULL ),
	m_pInternalTempTexture1( NULL ),
	m_pOutputTexture( NULL ),

	m_iInputWidth( -1 ),
	m_iInputHeight( -1 )
{

}

void DifferenceOfGaussiansEdgesKernel::deleteTextures()
{
	if( m_pInternalTempTexture1 != NULL )
	{
		delete m_pInternalTempTexture1;
		m_pInternalTempTexture1 = NULL;
	}

	if( m_pInternalTempTexture0 != NULL )
	{
		delete m_pInternalTempTexture0;
		m_pInternalTempTexture0 = NULL;
	}

	if( m_pOutputTexture != NULL )
	{
		delete m_pOutputTexture;
		m_pOutputTexture = NULL;
	}
}

// virtual
void DifferenceOfGaussiansEdgesKernel::reallocate()
{
	// inputs
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
		deleteTextures();

		int nBits = AppData::getInstance()->getTextureNumBits();
		int nComponents = AppData::getInstance()->getTextureNumComponents();

		if( nComponents == 1 )
		{
			m_pInternalTempTexture0 = GLTextureRectangle::createFloat1Texture( m_iInputWidth, m_iInputHeight, nBits );
			m_pInternalTempTexture1 = GLTextureRectangle::createFloat1Texture( m_iInputWidth, m_iInputHeight, nBits );

			m_pOutputTexture = GLTextureRectangle::createFloat1Texture( m_iInputWidth, m_iInputHeight, nBits );
		}
		else
		{
			m_pInternalTempTexture0 = GLTextureRectangle::createFloat4Texture( m_iInputWidth, m_iInputHeight, nBits );
			m_pInternalTempTexture1 = GLTextureRectangle::createFloat4Texture( m_iInputWidth, m_iInputHeight, nBits );

			m_pOutputTexture = GLTextureRectangle::createFloat4Texture( m_iInputWidth, m_iInputHeight, nBits );
		}

		m_pOutputTextureOutputPort->pushData( KernelPortData( m_pOutputTexture ) );
	}

	m_bReallocationNeeded = false;
}

void DifferenceOfGaussiansEdgesKernel::initializeCgPrograms()
{
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();

	//////////////////////////////////////////////////////////////////////////

	m_pGaussianBlurFragmentProgram = pProgramManager->getNamedProgram( "DoG_gaussianBlur" );
	m_cgp_GB_inputLabASampler = m_pGaussianBlurFragmentProgram->getNamedParameter( "inputLabASampler" );
	m_cgp_GB_f2Delta = m_pGaussianBlurFragmentProgram->getNamedParameter( "delta" );
	m_cgp_GB_f2SigmaTwoSigmaSquared = m_pGaussianBlurFragmentProgram->getNamedParameter( "sigmaTwoSigmaSquared" );

	//////////////////////////////////////////////////////////////////////////

	m_pGaussianBlurSubtractAndSmoothStepFragmentProgram = pProgramManager->getNamedProgram( "DoG_gaussianBlurSubtractAndSmoothStep" );
	m_cgp_GBSSS_largeXBlurredLabASampler = m_pGaussianBlurSubtractAndSmoothStepFragmentProgram->getNamedParameter( "largeXBlurredLabASampler" );
	m_cgp_GBSSS_smallBlurredLabASampler = m_pGaussianBlurSubtractAndSmoothStepFragmentProgram->getNamedParameter( "smallBlurredLabASampler" );
	m_cgp_GBSSS_f2Delta = m_pGaussianBlurSubtractAndSmoothStepFragmentProgram->getNamedParameter( "delta" );
	m_cgp_GBSSS_f2SigmaTwoSigmaSquared = m_pGaussianBlurSubtractAndSmoothStepFragmentProgram->getNamedParameter( "sigmaTwoSigmaSquared" );
	m_cgp_GBSSS_f2TauPhi = m_pGaussianBlurSubtractAndSmoothStepFragmentProgram->getNamedParameter( "tauPhi" );
}

void DifferenceOfGaussiansEdgesKernel::blurPass( float sigma,
												GLTextureRectangle* pInput, GLTextureRectangle* pOutput,
												float deltaX, float deltaY )
{
	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, pOutput );

	// setup input
	cgGLSetTextureParameter( m_cgp_GB_inputLabASampler,
		pInput->getTextureId() );

	// setup uniforms
	cgGLSetParameter2f( m_cgp_GB_f2Delta, deltaX, deltaY );
	cgGLSetParameter2f( m_cgp_GB_f2SigmaTwoSigmaSquared, sigma, 2 * sigma * sigma );

	m_pGaussianBlurFragmentProgram->bind();
	GLUtilities::drawQuad( m_iInputWidth, m_iInputHeight );
}

void DifferenceOfGaussiansEdgesKernel::blurYSubtractAndSmoothStepPass( float sigma,
																	  GLTextureRectangle* pInputLargeXBlurred,
																	  GLTextureRectangle* pInputSmallBlurred,
																	  GLTextureRectangle* pOutput,
																	  float deltaX, float deltaY )
{
	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, pOutput );	

	// setup inputs
	cgGLSetTextureParameter( m_cgp_GBSSS_largeXBlurredLabASampler,
		pInputLargeXBlurred->getTextureId() );
	cgGLSetTextureParameter( m_cgp_GBSSS_smallBlurredLabASampler,
		pInputSmallBlurred->getTextureId() );

	// setup uniforms
	cgGLSetParameter2f( m_cgp_GBSSS_f2Delta, deltaX, deltaY );
	cgGLSetParameter2f( m_cgp_GBSSS_f2SigmaTwoSigmaSquared, sigma, 2 * sigma * sigma );
	cgGLSetParameter2f( m_cgp_GBSSS_f2TauPhi, m_fTau, m_fPhi );

	m_pGaussianBlurSubtractAndSmoothStepFragmentProgram->bind();
	GLUtilities::drawQuad( m_iInputWidth, m_iInputHeight );
}
