#include "DetailMapKernel.h"

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
void DetailMapKernel::initializeCg()
{
	QString cgPrefix = AppData::getInstance()->getCgPathPrefix();
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	CGprofile latestCgFragmentProfile = CgShared::getInstance()->getLatestFragmentProfile();

	pProgramManager->loadProgramFromFile( "DM_makeDetailMap", cgPrefix + "DetailMap.cg",
		"makeDetailMap", latestCgFragmentProfile, NULL );

	s_bCgInitialized = true;
}

// static
DetailMapKernel* DetailMapKernel::create( QString args )
{
	if( !s_bCgInitialized )
	{
		initializeCg();
	}

	DetailMapKernel* pInstance = new DetailMapKernel;
	pInstance->initializeGL();
	return pInstance;
}

// virtual
DetailMapKernel::~DetailMapKernel()
{
	deleteOutputTextures();
}

// virtual
bool DetailMapKernel::isInputComplete()
{
	// HACK
	return true;
}

// virtual
void DetailMapKernel::makeDirty( QString inputPortName )
{
	if( inputPortName == "size" )
	{
		m_bReallocationNeeded = true;
	}

	m_pDetailMapOutputPort->makeDirty();
}

// virtual
void DetailMapKernel::compute( QString outputPortName )
{
	if( m_bReallocationNeeded )
	{
		reallocate();
	}

	// retrieve inputs	
	float sigma = m_pSigmaInputPort->pullData().getFloatData();
	float radius = m_pRadiusInputPort->pullData().getFloatData();

	// setup outputs
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pDetailMapTexture );			

	// set uniforms
	cgGLSetParameter2f( m_cgp_MDM_f2MouseXY, m_iMouseMovedX, m_iMouseMovedY );
	cgGLSetParameter2f( m_cgp_MDM_f2SigmaRadius, sigma, radius );	

	// render
	glClear( GL_COLOR_BUFFER_BIT );
	GLUtilities::setupOrthoCamera( m_qSize.width(), m_qSize.height() );
	m_pMakeDetailMapProgram->bind();
	GLUtilities::drawQuad( m_qSize.width(), m_qSize.height() );
}

//////////////////////////////////////////////////////////////////////////
// Public Slots
//////////////////////////////////////////////////////////////////////////

void DetailMapKernel::handleMousePressed( int x, int y, int button )
{

}

void DetailMapKernel::handleMouseMoved( int x, int y )
{
	m_iMouseMovedX = x;
	m_iMouseMovedY = y;

	m_pDetailMapOutputPort->makeDirty();
}

//////////////////////////////////////////////////////////////////////////
// Protected
//////////////////////////////////////////////////////////////////////////

// virtual
void DetailMapKernel::initializeGL()
{
	GPUKernel::initializeGL();
	initializeCgPrograms();
}

// virtual
void DetailMapKernel::initializePorts()
{
	// input ports
	m_pSizeInputPort = addInputPort( "size", KERNEL_PORT_DATA_TYPE_SIZE_2D );

	m_pSigmaInputPort = addInputPort( "sigma", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pSigmaInputPort->setFloatMinMaxDelta( 5.f, 1.f, 2048.f, 1.f );

	m_pRadiusInputPort = addInputPort( "radius", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pRadiusInputPort->setFloatMinMaxDelta( 5.f, 1.f, 2048.f, 1.f );

	// output ports
	m_pDetailMapOutputPort = addOutputPort( "detailMapTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

// static
bool DetailMapKernel::s_bCgInitialized = false;

DetailMapKernel::DetailMapKernel() :

	GPUKernel( "DetailMap" ),

	m_pFBO( GLShared::getInstance()->getSharedFramebufferObject() ),

	m_bReallocationNeeded( true ),

	// derived data
	// m_qSize is automatically invalid

	// outputs
	m_pDetailMapTexture( NULL ),

	// internal
	m_iMouseMovedX( 0 ),
	m_iMouseMovedY( 0 )
{

}

void DetailMapKernel::initializeCgPrograms()
{
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();

	//////////////////////////////////////////////////////////////////////////

	m_pMakeDetailMapProgram = pProgramManager->getNamedProgram( "DM_makeDetailMap" );
	m_cgp_MDM_f2MouseXY = m_pMakeDetailMapProgram->getNamedParameter( "mouseXY" );
	m_cgp_MDM_f2SigmaRadius = m_pMakeDetailMapProgram->getNamedParameter( "sigmaRadius" );
}

void DetailMapKernel::deleteOutputTextures()
{
	if( m_pDetailMapTexture != NULL )
	{
		delete m_pDetailMapTexture;
		m_pDetailMapTexture = NULL;
	}
}

void DetailMapKernel::reallocate()
{
	QSize size = m_pSizeInputPort->pullData().getSize2DData();
	if( size != m_qSize )
	{
		deleteOutputTextures();

		m_qSize = size;

		int nBits = AppData::getInstance()->getTextureNumBits();

		m_pDetailMapTexture = GLTextureRectangle::createFloat1Texture( m_qSize.width(), m_qSize.height(), nBits );
		m_pDetailMapOutputPort->pushData( KernelPortData( m_pDetailMapTexture ) );		
	}

	m_bReallocationNeeded = false;
}
