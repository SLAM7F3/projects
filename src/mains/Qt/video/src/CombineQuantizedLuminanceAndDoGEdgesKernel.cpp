#include "CombineQuantizedLuminanceAndDoGEdgesKernel.h"

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
void CombineQuantizedLuminanceAndDoGEdgesKernel::initializeCg()
{
	QString cgPrefix = AppData::getInstance()->getCgPathPrefix();
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	CGprofile latestCgFragmentProfile = CgShared::getInstance()->getLatestFragmentProfile();

	pProgramManager->loadProgramFromFile( "CQLADE_combineQuantizationAndEdges", cgPrefix + "CombineQuantizedLuminanceAndDoGEdges.cg",
		"combineQuantizationAndEdges", latestCgFragmentProfile, NULL );

	s_bCgInitialized = true;
}

// static
CombineQuantizedLuminanceAndDoGEdgesKernel* CombineQuantizedLuminanceAndDoGEdgesKernel::create( QString args )
{
	if( !s_bCgInitialized )
	{
		initializeCg();
	}

	CombineQuantizedLuminanceAndDoGEdgesKernel* pInstance = new CombineQuantizedLuminanceAndDoGEdgesKernel;
	pInstance->initializeGL();
	return pInstance;
}

// virtual
CombineQuantizedLuminanceAndDoGEdgesKernel::~CombineQuantizedLuminanceAndDoGEdgesKernel()
{
	deleteOutputTexture();
}

// virtual
bool CombineQuantizedLuminanceAndDoGEdgesKernel::isInputComplete()
{
	return true;
}


// virtual
void CombineQuantizedLuminanceAndDoGEdgesKernel::makeDirty( QString inputPortName )
{
	m_bReallocationNeeded = true;

	m_pOutputTextureOutputPort->makeDirty();
}

// virtual
void CombineQuantizedLuminanceAndDoGEdgesKernel::compute( QString outputPortName )
{
	if( m_bReallocationNeeded )
	{
		reallocate();
	}

	// retrieve inputs
	float gamma = m_pGammaInputPort->pullData().getFloatData();
	float saturation = m_pSaturationInputPort->pullData().getFloatData();

	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pOutputTexture );			

	// setup inputs
	cgGLSetTextureParameter( m_cgp_quantizedLabAInputSampler,
		m_pQuantizedLuminanceInputTexture->getTextureId() );
	cgGLSetTextureParameter( m_cgp_dogEdgesInputSampler,
		m_pDoGEdgesInputTexture->getTextureId() );
	cgGLSetTextureParameter( m_cgp_detailMapSampler,
		m_pDetailMapTexture->getTextureId() );
	
	// setup uniforms
	cgGLSetParameter2f( m_cgp_f2GammaSaturation,
		gamma, saturation );
	
	glClear( GL_COLOR_BUFFER_BIT );
	GLUtilities::setupOrthoCamera( m_iInputWidth, m_iInputHeight );
	m_pFragmentProgram->bind();
	GLUtilities::drawQuad( m_iInputWidth, m_iInputHeight );
}

//////////////////////////////////////////////////////////////////////////
// protected
//////////////////////////////////////////////////////////////////////////

// virtual
void CombineQuantizedLuminanceAndDoGEdgesKernel::initializeGL()
{
	GPUKernel::initializeGL();
	initializeCgPrograms();
}

// virtual					
void CombineQuantizedLuminanceAndDoGEdgesKernel::initializePorts()
{
	m_pQuantizedLuminanceInputPort = addInputPort( "lqTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
	m_pDoGEdgesInputPort = addInputPort( "edgeTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
	m_pDetailMapInputPort = addInputPort( "detailMapTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );

	m_pGammaInputPort = addInputPort( "gamma", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pGammaInputPort->setFloatMinMaxDelta( 1.f, 0.1f, 10.f, 0.1f );

	m_pSaturationInputPort = addInputPort( "saturation", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pSaturationInputPort->setFloatMinMaxDelta( 1.f, 0.f, 2.f, 0.1f );

	m_pOutputTextureOutputPort = addOutputPort( "outputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

// static
bool CombineQuantizedLuminanceAndDoGEdgesKernel::s_bCgInitialized = false;

CombineQuantizedLuminanceAndDoGEdgesKernel::CombineQuantizedLuminanceAndDoGEdgesKernel() :

	GPUKernel( "CombineQuantizedLuminanceAndDoGEdges" ),

	m_pFBO( GLShared::getInstance()->getSharedFramebufferObject() ),

	m_bReallocationNeeded( true ),

	m_pQuantizedLuminanceInputTexture( NULL ),
	m_pDoGEdgesInputTexture( NULL ),
	m_pOutputTexture( NULL ),

	m_iInputWidth( -1 ),
	m_iInputHeight( -1 )
{

}

void CombineQuantizedLuminanceAndDoGEdgesKernel::deleteOutputTexture()
{
	if( m_pOutputTexture != NULL )
	{
		delete m_pOutputTexture;
		m_pOutputTexture = NULL;
	}
}

void CombineQuantizedLuminanceAndDoGEdgesKernel::reallocate()
{
	// inputs
	m_pQuantizedLuminanceInputTexture = m_pQuantizedLuminanceInputPort->pullData().getGLTextureRectangleData();
	m_pDoGEdgesInputTexture = m_pDoGEdgesInputPort->pullData().getGLTextureRectangleData();

	// TODO: make gaze vs nogaze versions
	// commented out version is gaze
	// m_pDetailMapTexture = m_pDetailMapInputPort->pullData().getGLTextureRectangleData();

	// HACK: for nogaze
	m_pDetailMapTexture = m_pDoGEdgesInputPort->pullData().getGLTextureRectangleData();

	int qltWidth = m_pQuantizedLuminanceInputTexture->getWidth();
	int qltHeight = m_pQuantizedLuminanceInputTexture->getHeight();
	int dogWidth = m_pDoGEdgesInputTexture->getWidth();
	int dogHeight = m_pDoGEdgesInputTexture->getHeight();
	int dmWidth = m_pDetailMapTexture->getWidth();
	int dmHeight = m_pDetailMapTexture->getHeight();

	assert( qltWidth == dogWidth );
	assert( qltHeight == dogHeight );
	assert( dmWidth == dogWidth );
	assert( dmHeight == dogHeight );

	bool sizeChanged = false;
	if( m_iInputWidth != qltWidth )
	{
		m_iInputWidth = qltWidth;
		sizeChanged = true;
	}
	if( m_iInputHeight != qltHeight )
	{
		m_iInputHeight = qltHeight;
		sizeChanged = true;
	}

	if( sizeChanged )
	{
		deleteOutputTexture();
		m_pOutputTexture = GLTextureRectangle::createFloat4Texture( m_iInputWidth, m_iInputHeight, 16 );
		m_pOutputTextureOutputPort->pushData( KernelPortData( m_pOutputTexture ) );
	}

	m_bReallocationNeeded = false;
}

void CombineQuantizedLuminanceAndDoGEdgesKernel::initializeCgPrograms()
{
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();

	m_pFragmentProgram = pProgramManager->getNamedProgram( "CQLADE_combineQuantizationAndEdges" );
	m_cgp_quantizedLabAInputSampler = m_pFragmentProgram->getNamedParameter( "quantizedLabAInputSampler" );
	m_cgp_dogEdgesInputSampler = m_pFragmentProgram->getNamedParameter( "dogEdgesInputSampler" );
	m_cgp_detailMapSampler = m_pFragmentProgram->getNamedParameter( "detailMapSampler" );
	m_cgp_f2GammaSaturation = m_pFragmentProgram->getNamedParameter( "gammaSaturation" );
}
