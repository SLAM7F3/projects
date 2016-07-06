#include "RGB2LabKernel.h"

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
void RGB2LabKernel::initializeCg()
{
	QString cgPrefix = AppData::getInstance()->getCgPathPrefix();
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	CGprofile latestCgFragmentProfile = CgShared::getInstance()->getLatestFragmentProfile();

	pProgramManager->loadProgramFromFile( "RGB2Lab_convertRGBAToLabA", cgPrefix + "ColorSpaceConversion.cg",
		"convertRGBAToLabA", latestCgFragmentProfile, NULL );

	s_bCgInitialized = true;
}

// static
RGB2LabKernel* RGB2LabKernel::create( QString args )
{
	if( !s_bCgInitialized )
	{
		initializeCg();
	}

	RGB2LabKernel* pInstance = new RGB2LabKernel;
	pInstance->initializeGL();
	return pInstance;	
}

// virtual
RGB2LabKernel::~RGB2LabKernel()
{
	deleteOutputTexture();
}

// virtual
bool RGB2LabKernel::isInputComplete()
{
	return true;
}

// virtual
void RGB2LabKernel::makeDirty( QString inputPortName )
{
	if( inputPortName == "inputTexture" )
	{
		m_bReallocationNeeded = true;
	}

	m_pOutputTextureOutputPort->makeDirty();
}

// virtual
void RGB2LabKernel::compute( QString outputPortName )
{
	if( m_bReallocationNeeded )
	{
		reallocate();		
	}

	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pOutputTexture );	

	// setup input
	cgGLSetTextureParameter( m_cgp_F_inputSampler,
		m_pInputTexture->getTextureId() );

	glClear( GL_COLOR_BUFFER_BIT );
	GLUtilities::setupOrthoCamera( m_iInputWidth, m_iInputHeight );
	m_pFragmentProgram->bind();
	GLUtilities::drawQuad( m_iInputWidth, m_iInputHeight );
}

//////////////////////////////////////////////////////////////////////////
// protected
//////////////////////////////////////////////////////////////////////////

// virtual
void RGB2LabKernel::initializeGL()
{
	GPUKernel::initializeGL();
	initializeCgPrograms();
}

// virtual					
void RGB2LabKernel::initializePorts()
{	
	m_pInputTextureInputPort = addInputPort( "inputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
	m_pOutputTextureOutputPort = addOutputPort( "outputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

// static
bool RGB2LabKernel::s_bCgInitialized = false;

RGB2LabKernel::RGB2LabKernel() :

	GPUKernel( "RGB2Lab" ),

	m_pFBO( GLShared::getInstance()->getSharedFramebufferObject() ),

	m_bReallocationNeeded( true ),

	m_pInputTexture( NULL ),
	m_pOutputTexture( NULL ),

	m_iInputWidth( -1 ), // invalid
	m_iInputHeight( -1 ) // invalid

{

}

void RGB2LabKernel::deleteOutputTexture()
{
	if( m_pOutputTexture != NULL )
	{
		delete m_pOutputTexture;
		m_pOutputTexture = NULL;
	}
}

// virtual
void RGB2LabKernel::reallocate()
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
		deleteOutputTexture();
		int nBits = AppData::getInstance()->getTextureNumBits();
		m_pOutputTexture = GLTextureRectangle::createFloat4Texture( m_iInputWidth, m_iInputHeight, nBits );
		m_pOutputTextureOutputPort->pushData( KernelPortData( m_pOutputTexture ) );
	}

	m_bReallocationNeeded = false;
}

void RGB2LabKernel::initializeCgPrograms()
{
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	m_pFragmentProgram = pProgramManager->getNamedProgram( "RGB2Lab_convertRGBAToLabA" );
	m_cgp_F_inputSampler = m_pFragmentProgram->getNamedParameter( "RGBAInputSampler" );
}
