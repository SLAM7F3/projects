#include "GaussianBlurKernel.h"

#include <cassert>
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
void GaussianBlurKernel::initializeCg()
{
	QString cgPrefix = AppData::getInstance()->getCgPathPrefix();
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	CGprofile latestCgFragmentProfile = CgShared::getInstance()->getLatestFragmentProfile();

	/*
	pProgramManager->loadProgramFromFile( "GaussianBlur_downsample", cgPrefix + "GaussianBlur.cg",
		"downsample", latestCgFragmentProfile, NULL );

	pProgramManager->loadProgramFromFile( "GaussianBlur_blur", cgPrefix + "GaussianBlur.cg",
		"blur", latestCgFragmentProfile, NULL );

	pProgramManager->loadProgramFromFile( "GaussianBlur_upsample", cgPrefix + "GaussianBlur.cg",
		"upsample", latestCgFragmentProfile, NULL );
	*/

	pProgramManager->loadProgramFromFile( "GaussianBlur_gaussianBlur", cgPrefix + "GaussianBlur.cg",
		"gaussianBlur", latestCgFragmentProfile, NULL );

	pProgramManager->loadProgramFromFile( "GaussianBlur_absHighPass", cgPrefix + "GaussianBlur.cg",
		"absHighPass", latestCgFragmentProfile, NULL );

	s_bCgInitialized = true;
}

// static
GaussianBlurKernel* GaussianBlurKernel::create( QString args )
{
	if( !s_bCgInitialized )
	{
		initializeCg();
	}

	GaussianBlurKernel* pInstance = new GaussianBlurKernel;
	pInstance->initializeGL();
	return pInstance;	
}

// virtual
GaussianBlurKernel::~GaussianBlurKernel()
{
	cleanup();
}

// virtual
bool GaussianBlurKernel::isInputComplete()
{
	return true;
}

// virtual
void GaussianBlurKernel::makeDirty( QString inputPortName )
{
	if( inputPortName == "inputTexture" )
	{
		m_bReallocationNeeded = true;
	}

	m_pOutputTextureOutputPort->makeDirty();
}

// virtual
void GaussianBlurKernel::compute( QString outputPortName )
{
	if( m_bReallocationNeeded )
	{
		reallocate();		
	}

#if 0
	downsamplePass();
	blurPass();
	upsamplePass();
#endif

	m_fSigma = m_pSigmaInputPort->pullData().getFloatData();

	//////////////////////////////////////////////////////////////////////////
	// blur x, temp0 --> temp1
	//////////////////////////////////////////////////////////////////////////	

	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pTempTexture0 );

	// setup input	
	cgGLSetTextureParameter( m_cgp_gb_inputSampler,
		m_pInputTexture->getTextureId() );
	cgGLSetParameter2f( m_cgp_gb_f2_Delta, 1, 0 );
	cgGLSetParameter2f( m_cgp_gb_f2_sigmaTwoSigmaSquared, m_fSigma, 2 * m_fSigma * m_fSigma );

	// render
	glClear( GL_COLOR_BUFFER_BIT );
	GLUtilities::setupOrthoCamera( m_iInputWidth, m_iInputHeight );
	m_pGaussianBlurFragmentProgram->bind();
	GLUtilities::drawQuad( 0, 0, m_iInputWidth, m_iInputHeight );

	//////////////////////////////////////////////////////////////////////////
	// blur y, temp0 --> output
	//////////////////////////////////////////////////////////////////////////

	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pOutputTexture );

	// setup input	
	cgGLSetTextureParameter( m_cgp_ahp_inputSampler,
		m_pInputTexture->getTextureId() );
	cgGLSetTextureParameter( m_cgp_ahp_blurredOnceSampler,
		m_pTempTexture0->getTextureId() );
	cgGLSetParameter2f( m_cgp_ahp_f2_Delta, 0, 1 );
	cgGLSetParameter2f( m_cgp_ahp_f2_sigmaTwoSigmaSquared, m_fSigma, 2 * m_fSigma * m_fSigma );

	// render
	glClear( GL_COLOR_BUFFER_BIT );
	GLUtilities::setupOrthoCamera( m_iInputWidth, m_iInputHeight );
	m_pAbsHighPassFragmentProgram->bind();
	GLUtilities::drawQuad( 0, 0, m_iInputWidth, m_iInputHeight );
}

//////////////////////////////////////////////////////////////////////////
// protected
//////////////////////////////////////////////////////////////////////////

// virtual
void GaussianBlurKernel::initializeGL()
{
	GPUKernel::initializeGL();
	initializeCgPrograms();
}

// virtual					
void GaussianBlurKernel::initializePorts()
{	
	m_pInputTextureInputPort = addInputPort( "inputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
	m_pSigmaInputPort = addInputPort( "sigma", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pSigmaInputPort->setFloatMinMaxDelta( 5.f, 1.f, 100.f, 1.f );

	m_pOutputTextureOutputPort = addOutputPort( "outputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

// static
bool GaussianBlurKernel::s_bCgInitialized = false;

GaussianBlurKernel::GaussianBlurKernel() :

	GPUKernel( "GaussianBlur" ),

	m_pFBO( GLShared::getInstance()->getSharedFramebufferObject() ),

	m_bReallocationNeeded( true ),

	m_pInputTexture( NULL ),
	m_pOutputTexture( NULL ),

	m_pTempTexture0( NULL ),
	m_pTempTexture1( NULL ),

	m_iInputWidth( -1 ), // invalid
	m_iInputHeight( -1 ), // invalid
	m_iDownsampledWidth( -1 ), // invalid
	m_iDownsampledHeight( -1 ), // invalid
	m_fSigma( -1 ) // invalid
{

}

void GaussianBlurKernel::cleanup()
{
	if( m_pTempTexture1 != NULL )
	{
		delete m_pTempTexture1;
		m_pTempTexture1 = NULL;
	}

	if( m_pTempTexture0 != NULL )
	{
		delete m_pTempTexture0;
		m_pTempTexture0 = NULL;
	}

	if( m_pOutputTexture != NULL )
	{
		delete m_pOutputTexture;
		m_pOutputTexture = NULL;
	}
}

// virtual
void GaussianBlurKernel::reallocate()
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

	int nBits = AppData::getInstance()->getTextureNumBits();

	if( sizeChanged )
	{
		cleanup();

		// allocate output		
		m_pOutputTexture = GLTextureRectangle::createFloat4Texture( m_iInputWidth, m_iInputHeight, nBits );
		m_pOutputTextureOutputPort->pushData( KernelPortData( m_pOutputTexture ) );

		// allocate temp
		m_pTempTexture0 = GLTextureRectangle::createFloat4Texture( m_iInputWidth, m_iInputHeight, nBits );
	}

#if 0
	int downsampledWidth = static_cast< int >( ( m_iInputWidth - 1 ) / m_fSigma ) + 1;
	int downsampledHeight = static_cast< int >( ( m_iInputHeight - 1 ) / m_fSigma ) + 1;

	if( downsampledWidth != m_iDownsampledWidth || downsampledHeight != m_iDownsampledHeight )
	{
		m_iDownsampledWidth = downsampledWidth;
		m_iDownsampledHeight = downsampledHeight;

		// allocate temp
		m_pTempTexture0 = GLTextureRectangle::createFloat4Texture( m_iDownsampledWidth, m_iDownsampledHeight, nBits );
		m_pTempTexture0->setWrapMode( GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );

		m_pTempTexture1 = GLTextureRectangle::createFloat4Texture( m_iDownsampledWidth, m_iDownsampledHeight, nBits );
		m_pTempTexture1->setWrapMode( GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );
	}
#endif

	m_bReallocationNeeded = false;
}

void GaussianBlurKernel::initializeCgPrograms()
{
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();

#if 0
	m_pDownsampleFragmentProgram = pProgramManager->getNamedProgram( "GaussianBlur_downsample" );
	m_cgp_downsample_inputSampler = m_pDownsampleFragmentProgram->getNamedParameter( "inputSampler" );
	m_cgp_downsample_sigma = m_pDownsampleFragmentProgram->getNamedParameter( "sigma" );

	m_pBlurFragmentProgram = pProgramManager->getNamedProgram( "GaussianBlur_blur" );
	m_cgp_blur_inputSampler = m_pBlurFragmentProgram->getNamedParameter( "inputSampler" );
	m_cgp_blur_delta_twoDelta = m_pBlurFragmentProgram->getNamedParameter( "delta_twoDelta" );

	m_pUpsampleFragmentProgram = pProgramManager->getNamedProgram( "GaussianBlur_upsample" );
	m_cgp_upsample_inputSampler = m_pUpsampleFragmentProgram->getNamedParameter( "inputSampler" );
	m_cgp_upsample_downsampledSampler = m_pUpsampleFragmentProgram->getNamedParameter( "downsampledSampler" );
	m_cgp_upsample_fSigma = m_pUpsampleFragmentProgram->getNamedParameter( "sigma" );
#endif

	m_pGaussianBlurFragmentProgram = pProgramManager->getNamedProgram( "GaussianBlur_gaussianBlur" );
	m_cgp_gb_inputSampler = m_pGaussianBlurFragmentProgram->getNamedParameter( "inputSampler" );
	m_cgp_gb_f2_Delta = m_pGaussianBlurFragmentProgram->getNamedParameter( "delta" );
	m_cgp_gb_f2_sigmaTwoSigmaSquared = m_pGaussianBlurFragmentProgram->getNamedParameter( "sigmaTwoSigmaSquared" );

	m_pAbsHighPassFragmentProgram = pProgramManager->getNamedProgram( "GaussianBlur_absHighPass" );
	m_cgp_ahp_inputSampler = m_pAbsHighPassFragmentProgram->getNamedParameter( "inputSampler" );
	m_cgp_ahp_blurredOnceSampler = m_pAbsHighPassFragmentProgram->getNamedParameter( "blurredOnceSampler" );
	m_cgp_ahp_f2_Delta = m_pAbsHighPassFragmentProgram->getNamedParameter( "delta" );
	m_cgp_ahp_f2_sigmaTwoSigmaSquared = m_pAbsHighPassFragmentProgram->getNamedParameter( "sigmaTwoSigmaSquared" );	
}

#if 0

void GaussianBlurKernel::downsamplePass()
{
	// setup output	
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pTempTexture0 );

	// setup input
	m_pInputTexture->setFilterMode( GLTexture::FILTER_MODE_LINEAR, GLTexture::FILTER_MODE_LINEAR );

	cgGLSetParameter1f( m_cgp_downsample_sigma, m_fSigma );	
	cgGLSetTextureParameter( m_cgp_downsample_inputSampler,
		m_pInputTexture->getTextureId() );

	// render
	glClear( GL_COLOR_BUFFER_BIT );
	GLUtilities::setupOrthoCamera( m_iDownsampledWidth, m_iDownsampledHeight );
	m_pDownsampleFragmentProgram->bind();
	GLUtilities::drawQuad( 0, 0, m_iDownsampledWidth, m_iDownsampledHeight );

	// reset state
	m_pInputTexture->setFilterMode( GLTexture::FILTER_MODE_NEAREST, GLTexture::FILTER_MODE_NEAREST );

	// m_pTempTexture0->dumpToPNG( "d:/_downsampled.png" );
}

void GaussianBlurKernel::blurPass()
{
	//////////////////////////////////////////////////////////////////////////
	// blur x, temp0 --> temp1
	//////////////////////////////////////////////////////////////////////////

	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pTempTexture1 );

	// setup input
	cgGLSetParameter4f( m_cgp_blur_delta_twoDelta, 1, 0, 2, 0 );
	cgGLSetTextureParameter( m_cgp_blur_inputSampler,
		m_pTempTexture0->getTextureId() );

	// render
	glClear( GL_COLOR_BUFFER_BIT );	
	m_pBlurFragmentProgram->bind();
	GLUtilities::drawQuad( 0, 0, m_iDownsampledWidth, m_iDownsampledHeight );

	//////////////////////////////////////////////////////////////////////////
	// blur y, temp1 --> temp0
	//////////////////////////////////////////////////////////////////////////

	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pTempTexture0 );

	// setup input
	cgGLSetParameter4f( m_cgp_blur_delta_twoDelta, 0, 1, 0, 2 );
	cgGLSetTextureParameter( m_cgp_blur_inputSampler,
		m_pTempTexture1->getTextureId() );

	// render
	glClear( GL_COLOR_BUFFER_BIT );	
	m_pBlurFragmentProgram->bind();
	GLUtilities::drawQuad( 0, 0, m_iDownsampledWidth, m_iDownsampledHeight );

	// m_pTempTexture0->dumpToPNG( "d:/_downsampled_blurred.png" );
}

void GaussianBlurKernel::upsamplePass()
{
	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pOutputTexture );

	// setup input
	m_pTempTexture0->setFilterMode( GLTexture::FILTER_MODE_LINEAR, GLTexture::FILTER_MODE_LINEAR );

	cgGLSetParameter1f( m_cgp_upsample_fSigma, m_fSigma );
	cgGLSetTextureParameter( m_cgp_upsample_inputSampler,
		m_pInputTexture->getTextureId() );
	cgGLSetTextureParameter( m_cgp_upsample_downsampledSampler,
		m_pTempTexture0->getTextureId() );

	// render
	glClear( GL_COLOR_BUFFER_BIT );
	GLUtilities::setupOrthoCamera( m_iInputWidth, m_iInputHeight );
	m_pUpsampleFragmentProgram->bind();
	GLUtilities::drawQuad( 0, 0, m_iInputWidth, m_iInputHeight );

	// reset state
	m_pTempTexture0->setFilterMode( GLTexture::FILTER_MODE_NEAREST, GLTexture::FILTER_MODE_NEAREST );

	// m_pOutputTexture->dumpToPNG( "d:/_upsampled.png" );
	// exit( -1 );
}
#endif
