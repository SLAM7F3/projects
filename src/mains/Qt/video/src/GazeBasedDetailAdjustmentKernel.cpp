#include "GazeBasedDetailAdjustmentKernel.h"

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
void GazeBasedDetailAdjustmentKernel::initializeCg()
{
	QString cgPrefix = AppData::getInstance()->getCgPathPrefix();
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	CGprofile latestCgFragmentProfile = CgShared::getInstance()->getLatestFragmentProfile();

	pProgramManager->loadProgramFromFile( "GBDA_combineBFOutputs", cgPrefix + "GazeBasedDetailAdjustment.cg",
		"combineBFOutputs", latestCgFragmentProfile, NULL );

	pProgramManager->loadProgramFromFile( "GBDA_overlayDetailMapOverOriginal", cgPrefix + "GazeBasedDetailAdjustment.cg",
		"overlayDetailMapOverOriginal", latestCgFragmentProfile, NULL );	

	s_bCgInitialized = true;
}

// static
GazeBasedDetailAdjustmentKernel* GazeBasedDetailAdjustmentKernel::create( QString args )
{
	if( !s_bCgInitialized )
	{
		initializeCg();
	}

	GazeBasedDetailAdjustmentKernel* pInstance = new GazeBasedDetailAdjustmentKernel;
	pInstance->initializeGL();
	return pInstance;
}

// virtual
GazeBasedDetailAdjustmentKernel::~GazeBasedDetailAdjustmentKernel()
{
	deleteOutputTextures();
}

// virtual
bool GazeBasedDetailAdjustmentKernel::isInputComplete()
{
	// HACK
	return true;
}

// virtual
void GazeBasedDetailAdjustmentKernel::makeDirty( QString inputPortName )
{
	if( inputPortName == "originalLabTexture" ||
		inputPortName == "bilateralFilterTextures" ||
		inputPortName == "outputOverlayTexture" )
	{
		m_bReallocationNeeded = true;
	}

	m_pOutputTextureOutputPort->makeDirty();
	m_pOutputOverlayOutputPort->makeDirty();
}

// virtual
void GazeBasedDetailAdjustmentKernel::compute( QString outputPortName )
{
	if( m_bReallocationNeeded )
	{
		reallocate();
	}

	if( outputPortName == "outputOverlayTexture" )
	{
		detailMapPass();
	}
	else
	{
		combineBFPass();
	}
}

//////////////////////////////////////////////////////////////////////////
// Protected
//////////////////////////////////////////////////////////////////////////

// virtual
void GazeBasedDetailAdjustmentKernel::initializeGL()
{
	GPUKernel::initializeGL();
	initializeCgPrograms();
}

// virtual
void GazeBasedDetailAdjustmentKernel::initializePorts()
{
	// input ports
	m_pDetailOverlayAlphaInputPort = addInputPort( "detailOverlayAlpha", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pDetailOverlayAlphaInputPort->setFloatMinMaxDelta( 0.5f, 0.f, 1.f, 0.05f );

	m_pInputTextureInputPort = addInputPort( "inputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
	m_pBilateralFilterTexturesInputPort = addInputPort( "bilateralFilterTextures", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE_ARRAY );
	m_pDetailMapInputPort = addInputPort( "detailMapTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );

	// output ports
	m_pOutputTextureOutputPort = addOutputPort( "outputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
	m_pOutputOverlayOutputPort = addOutputPort( "outputOverlayTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

// static
bool GazeBasedDetailAdjustmentKernel::s_bCgInitialized = false;

GazeBasedDetailAdjustmentKernel::GazeBasedDetailAdjustmentKernel() :

	GPUKernel( "GazeBasedDetailAdjustment" ),

	m_pFBO( GLShared::getInstance()->getSharedFramebufferObject() ),

	m_bReallocationNeeded( true ),

	// derived data
	m_iWidth( -1 ), // invalid
	m_iHeight( -1 ), // invalid

	// outputs
	m_pOutputTexture( NULL ),
	m_pOutputOverlayTexture( NULL ),

	// internal
	m_iMouseMovedX( 0 ),
	m_iMouseMovedY( 0 )
{

}

void GazeBasedDetailAdjustmentKernel::initializeCgPrograms()
{
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();

	//////////////////////////////////////////////////////////////////////////

	m_pCombineBFFragmentProgram = pProgramManager->getNamedProgram( "GBDA_combineBFOutputs" );

	m_cgp_CBF_inputRGBASampler = m_pCombineBFFragmentProgram->getNamedParameter( "inputRGBASampler" );
	m_cgp_CBF_bf0Sampler = m_pCombineBFFragmentProgram->getNamedParameter( "bf0Sampler" );
	m_cgp_CBF_bf1Sampler = m_pCombineBFFragmentProgram->getNamedParameter( "bf1Sampler" );
	m_cgp_CBF_bf2Sampler = m_pCombineBFFragmentProgram->getNamedParameter( "bf2Sampler" );
	m_cgp_CBF_detailMapSampler = m_pCombineBFFragmentProgram->getNamedParameter( "detailMapSampler" );

	//////////////////////////////////////////////////////////////////////////

	m_pOverlayDetailMapOverOriginalFragmentProgram = pProgramManager->getNamedProgram( "GBDA_overlayDetailMapOverOriginal" );

	m_cgp_ODMOO_inputRGBASampler = m_pOverlayDetailMapOverOriginalFragmentProgram->getNamedParameter( "inputRGBASampler" );
	m_cgp_ODMOO_detailMapSampler = 	m_pOverlayDetailMapOverOriginalFragmentProgram->getNamedParameter( "detailMapSampler" );
	m_cgp_ODMOO_fDetailOverlayAlpha = m_pOverlayDetailMapOverOriginalFragmentProgram->getNamedParameter( "detailOverlayAlpha" );
}

void GazeBasedDetailAdjustmentKernel::deleteOutputTextures()
{
	if( m_pOutputOverlayTexture != NULL )
	{
		delete m_pOutputOverlayTexture;
		m_pOutputOverlayTexture = NULL;
	}

	if( m_pOutputTexture != NULL )
	{
		delete m_pOutputTexture;
		m_pOutputTexture = NULL;
	}
}

void GazeBasedDetailAdjustmentKernel::reallocate()
{
	m_pInputTexture = m_pInputTextureInputPort->pullData().getGLTextureRectangleData();
	m_qvBFTextures = m_pBilateralFilterTexturesInputPort->pullData().getGLTextureRectangleArrayData();
	m_pDetailMapTexture = m_pDetailMapInputPort->pullData().getGLTextureRectangleData();

	assert( m_qvBFTextures.size() >= 3 );

	bool sizeChanged = false;
	if( m_iWidth != m_pInputTexture->getWidth() )
	{
		m_iWidth = m_pInputTexture->getWidth();
		sizeChanged = true;
	}
	if( m_iHeight != m_pInputTexture->getHeight() )
	{
		m_iHeight = m_pInputTexture->getHeight();
		sizeChanged = true;
	}

	assert( m_qvBFTextures.at( 0 )->getWidth() == m_iWidth );
	assert( m_qvBFTextures.at( 0 )->getHeight() == m_iHeight );
	assert( m_pDetailMapTexture->getWidth() == m_iWidth );
	assert( m_pDetailMapTexture->getHeight() == m_iHeight );

	if( sizeChanged )
	{
		deleteOutputTextures();

		int nBits = AppData::getInstance()->getTextureNumBits();

		m_pOutputTexture = GLTextureRectangle::createFloat4Texture( m_iWidth, m_iHeight, nBits );
		m_pOutputTextureOutputPort->pushData( KernelPortData( m_pOutputTexture ) );

		m_pOutputOverlayTexture = GLTextureRectangle::createFloat4Texture( m_iWidth, m_iHeight, nBits );
		m_pOutputOverlayOutputPort->pushData( KernelPortData( m_pOutputOverlayTexture ) );		
	}

	m_bReallocationNeeded = false;
}

void GazeBasedDetailAdjustmentKernel::combineBFPass()
{
	// retrieve inputs

	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pOutputTexture );			

	// setup inputs
	cgGLSetTextureParameter( m_cgp_CBF_inputRGBASampler,
		m_pInputTexture->getTextureId() );
	cgGLSetTextureParameter( m_cgp_CBF_bf0Sampler,
		m_qvBFTextures.at( 0 )->getTextureId() );
	cgGLSetTextureParameter( m_cgp_CBF_bf1Sampler,
		m_qvBFTextures.at( 1 )->getTextureId() );
	cgGLSetTextureParameter( m_cgp_CBF_bf2Sampler,
		m_qvBFTextures.at( 2 )->getTextureId() );
	cgGLSetTextureParameter( m_cgp_CBF_detailMapSampler,
		m_pDetailMapTexture->getTextureId() );

	// set uniforms

	// render
	glClear( GL_COLOR_BUFFER_BIT );
	GLUtilities::setupOrthoCamera( m_iWidth, m_iHeight );
	m_pCombineBFFragmentProgram->bind();
	GLUtilities::drawQuad( m_iWidth, m_iHeight );
}

void GazeBasedDetailAdjustmentKernel::detailMapPass()
{
	// retrieve inputs	
	float detailOverlayAlpha = m_pDetailOverlayAlphaInputPort->pullData().getFloatData();

	// setup outputs
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pOutputOverlayTexture );

	// setup inputs
	cgGLSetTextureParameter( m_cgp_ODMOO_inputRGBASampler,
		m_pInputTexture->getTextureId() );
	cgGLSetTextureParameter( m_cgp_ODMOO_detailMapSampler,
		m_pDetailMapTexture->getTextureId() );
	
	// set uniforms
	cgGLSetParameter1f( m_cgp_ODMOO_fDetailOverlayAlpha, detailOverlayAlpha );

	// render
	glClear( GL_COLOR_BUFFER_BIT );
	GLUtilities::setupOrthoCamera( m_iWidth, m_iHeight );
	m_pOverlayDetailMapOverOriginalFragmentProgram->bind();
	GLUtilities::drawQuad( m_iWidth, m_iHeight );
}
