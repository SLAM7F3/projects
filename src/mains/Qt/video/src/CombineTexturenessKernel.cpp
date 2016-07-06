#include "CombineTexturenessKernel.h"

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
void CombineTexturenessKernel::initializeCg()
{
	QString cgPrefix = AppData::getInstance()->getCgPathPrefix();
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	CGprofile latestCgFragmentProfile = CgShared::getInstance()->getLatestFragmentProfile();

	pProgramManager->loadProgramFromFile( "CombineTextureness_combine", cgPrefix + "PerPixelProcessor.cg",
		"combine", latestCgFragmentProfile, NULL );

	s_bCgInitialized = true;
}

// static
CombineTexturenessKernel* CombineTexturenessKernel::create( QString args )
{
	if( !s_bCgInitialized )
	{
		initializeCg();
	}

	CombineTexturenessKernel* pInstance = new CombineTexturenessKernel;
	pInstance->initializeGL();
	return pInstance;	
}

// virtual
CombineTexturenessKernel::~CombineTexturenessKernel()
{
	deleteOutputTexture();
}

// virtual
bool CombineTexturenessKernel::isInputComplete()
{
	return true;
}

// virtual
void CombineTexturenessKernel::makeDirty( QString inputPortName )
{
	m_bReallocationNeeded = true;
	m_pOutputTextureOutputPort->makeDirty();
}

// virtual
void CombineTexturenessKernel::compute( QString outputPortName )
{
	if( m_bReallocationNeeded )
	{
		reallocate();		
	}

	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pOutputTexture );	

	// setup input
	cgGLSetTextureParameter( m_cgp_F_basePrimeSampler,
		m_pBasePrimeTexture->getTextureId() );
	cgGLSetTextureParameter( m_cgp_F_detailSampler,
		m_pDetailTexture->getTextureId() );
	cgGLSetTextureParameter( m_cgp_F_texturenessPrimeSampler,
		m_pTexturenessPrimeTexture->getTextureId() );
	cgGLSetTextureParameter( m_cgp_F_texturenessBasePrimeSampler,
		m_pTexturenessBasePrimeTexture->getTextureId() );
	cgGLSetTextureParameter( m_cgp_F_texturenessDetailSampler,
		m_pTexturenessDetailTexture->getTextureId() );

	glClear( GL_COLOR_BUFFER_BIT );
	GLUtilities::setupOrthoCamera( m_iInputWidth, m_iInputHeight );
	m_pFragmentProgram->bind();
	GLUtilities::drawQuad( m_iInputWidth, m_iInputHeight );
}

//////////////////////////////////////////////////////////////////////////
// protected
//////////////////////////////////////////////////////////////////////////

// virtual
void CombineTexturenessKernel::initializeGL()
{
	GPUKernel::initializeGL();
	initializeCgPrograms();
}

// virtual					
void CombineTexturenessKernel::initializePorts()
{	
	m_pBasePrimeInputPort = addInputPort( "basePrimeTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
	m_pDetailInputPort = addInputPort( "detailTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
	m_pTexturenessPrimeInputPort = addInputPort( "texturenessPrimeTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
	m_pTexturenessBasePrimeInputPort = addInputPort( "texturenessBasePrimeTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
	m_pTexturenessDetailInputPort = addInputPort( "texturenessDetailTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );

	m_pOutputTextureOutputPort = addOutputPort( "outputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

// static
bool CombineTexturenessKernel::s_bCgInitialized = false;

CombineTexturenessKernel::CombineTexturenessKernel() :

	GPUKernel( "CombineTextureness" ),

	m_pFBO( GLShared::getInstance()->getSharedFramebufferObject() ),

	m_bReallocationNeeded( true ),

	m_pBasePrimeTexture( NULL ),
	m_pDetailTexture( NULL ),
	m_pTexturenessPrimeTexture( NULL ),
	m_pTexturenessBasePrimeTexture( NULL ),
	m_pTexturenessDetailTexture( NULL ),
	m_pOutputTexture( NULL ),

	m_iInputWidth( -1 ), // invalid
	m_iInputHeight( -1 ) // invalid

{

}

void CombineTexturenessKernel::deleteOutputTexture()
{
	if( m_pOutputTexture != NULL )
	{
		delete m_pOutputTexture;
		m_pOutputTexture = NULL;
	}
}

// virtual
void CombineTexturenessKernel::reallocate()
{
	// inputs
	m_pBasePrimeTexture = m_pBasePrimeInputPort->pullData().getGLTextureRectangleData();
	m_pDetailTexture = m_pDetailInputPort->pullData().getGLTextureRectangleData();
	m_pTexturenessPrimeTexture = m_pTexturenessPrimeInputPort->pullData().getGLTextureRectangleData();
	m_pTexturenessBasePrimeTexture = m_pTexturenessBasePrimeInputPort->pullData().getGLTextureRectangleData();
	m_pTexturenessDetailTexture = m_pTexturenessDetailInputPort->pullData().getGLTextureRectangleData();

	bool sizeChanged = false;
	if( m_iInputWidth != m_pBasePrimeTexture->getWidth() )
	{
		m_iInputWidth = m_pBasePrimeTexture->getWidth();
		sizeChanged = true;
	}
	if( m_iInputHeight != m_pBasePrimeTexture->getHeight() )
	{
		m_iInputHeight = m_pBasePrimeTexture->getHeight();
		sizeChanged = true;
	}

	if( sizeChanged )
	{
		deleteOutputTexture();
		int nBits = AppData::getInstance()->getTextureNumBits();
		m_pOutputTexture = GLTextureRectangle::createFloat4Texture( m_iInputWidth, m_iInputHeight, nBits );
		m_pOutputTextureOutputPort->pushData( KernelPortData( m_pOutputTexture ) );
	}

	m_bReallocationNeeded = false;
}

void CombineTexturenessKernel::initializeCgPrograms()
{
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	m_pFragmentProgram = pProgramManager->getNamedProgram( "CombineTextureness_combine" );
	
	m_cgp_F_basePrimeSampler = m_pFragmentProgram->getNamedParameter( "basePrimeSampler" );
	m_cgp_F_detailSampler = m_pFragmentProgram->getNamedParameter( "detailSampler" );
	m_cgp_F_texturenessPrimeSampler = m_pFragmentProgram->getNamedParameter( "texturenessPrimeSampler" );
	m_cgp_F_texturenessBasePrimeSampler = m_pFragmentProgram->getNamedParameter( "texturenessBasePrimeSampler" );
	m_cgp_F_texturenessDetailSampler = m_pFragmentProgram->getNamedParameter( "texturenessDetailSampler" );
}
