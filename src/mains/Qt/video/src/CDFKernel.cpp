#include "CDFKernel.h"

#include <cassert>
#include <Cg/CgProgramManager.h>
#include <Cg/CgProgramWrapper.h>
#include <Cg/CgShared.h>
#include <Cg/CgUtilities.h>
#include <GL/GLFramebufferObject.h>
#include <GL/GLShared.h>
#include <GL/GLTextureRectangle.h>
#include <GL/GLUtilities.h>
#include <GL/GLBufferObject.h>

#include "AppData.h"

//////////////////////////////////////////////////////////////////////////
// Public
//////////////////////////////////////////////////////////////////////////

// static
void CDFKernel::initializeCg()
{
	QString cgPrefix = AppData::getInstance()->getCgPathPrefix();
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	CGprofile latestCgFragmentProfile = CgShared::getInstance()->getLatestFragmentProfile();

	pProgramManager->loadProgramFromFile( "CDF_accumulateCDF", cgPrefix + "Histogram.cg",
		"accumulateCDF", latestCgFragmentProfile, NULL );

	s_bCgInitialized = true;
}

// static
CDFKernel* CDFKernel::create( QString args )
{
	if( !s_bCgInitialized )
	{
		initializeCg();
	}

	CDFKernel* pInstance = new CDFKernel( args );
	pInstance->initializeGL();
	return pInstance;
}

// virtual
CDFKernel::~CDFKernel()
{	
	deleteOutputTexture();
}

// virtual
bool CDFKernel::isInputComplete()
{
	return true;
}

// virtual
void CDFKernel::makeDirty( QString inputPortName )
{
	if( inputPortName == "inputTexture" )
	{
		m_bReallocationNeeded = true;
	}
	m_pOutputPort->makeDirty();
}

// virtual
void CDFKernel::compute( QString outputPortName )
{
	if( m_bReallocationNeeded )
	{
		reallocate();
	}

	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pOutputTexture );

	// setup input
	cgGLSetTextureParameter( m_cgp_F_histogramSampler, m_pInputTexture->getTextureId() );

	// draw
	glClear( GL_COLOR_BUFFER_BIT );
	GLUtilities::setupOrthoCamera( m_nBins, 1 );
	m_pFragmentProgram->bind();
	GLUtilities::drawQuad( m_nBins, 1 );
}

//////////////////////////////////////////////////////////////////////////
// protected
//////////////////////////////////////////////////////////////////////////

// virtual
void CDFKernel::initializeGL()
{
	GPUKernel::initializeGL();
	initializeCgPrograms();
}

// virtual					
void CDFKernel::initializePorts()
{
	// TODO: oversampling factor
	m_pInputTextureInputPort = addInputPort( "inputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );

	m_pOutputPort = addOutputPort( "outputTexture",	KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

// static
bool CDFKernel::s_bCgInitialized = false;

CDFKernel::CDFKernel( QString args ) :

	GPUKernel( "CDF" ),
	m_pFBO( GLShared::getInstance()->getSharedFramebufferObject() ),

	m_bReallocationNeeded( true ),

	m_nBins( -1 ), // invalid

	// outputs
	m_pOutputTexture( NULL )
{

}

void CDFKernel::deleteOutputTexture()
{
	if( m_pOutputTexture != NULL )
	{
		delete m_pOutputTexture;
		m_pOutputTexture = NULL;
	}
}

void CDFKernel::reallocate()
{
	m_pInputTexture = m_pInputTextureInputPort->pullData().getGLTextureRectangleData();
	int nBins = m_pInputTexture->getWidth();
	if( nBins != m_nBins )
	{
		m_nBins = nBins;
		deleteOutputTexture();

		int nBits = AppData::getInstance()->getTextureNumBits();

		// TODO: MUCH more accurate with 32-bit precision		
		m_pOutputTexture = GLTextureRectangle::createFloat1Texture( m_nBins, 1, nBits );
		m_pOutputPort->pushData( KernelPortData( m_pOutputTexture ) );		
	}

	m_bReallocationNeeded = false;
}

void CDFKernel::initializeCgPrograms()
{
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();

	m_pFragmentProgram = pProgramManager->getNamedProgram( "CDF_accumulateCDF" );
	m_cgp_F_histogramSampler = m_pFragmentProgram->getNamedParameter( "histogramSampler" );
}
