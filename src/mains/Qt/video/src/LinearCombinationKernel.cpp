#include "LinearCombinationKernel.h"

#include <cassert>
#include <Cg/CgProgramManager.h>
#include <Cg/CgProgramWrapper.h>
#include <Cg/CgShared.h>
#include <Cg/CgUtilities.h>
#include <GL/GLFramebufferObject.h>
#include <GL/GLShared.h>
#include <GL/GLTextureRectangle.h>
#include <GL/GLUtilities.h>
#include <QStringList>

#include "AppData.h"

//////////////////////////////////////////////////////////////////////////
// Public
//////////////////////////////////////////////////////////////////////////

// static
void LinearCombinationKernel::initializeCg()
{
	QString cgPrefix = AppData::getInstance()->getCgPathPrefix();
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	CGprofile latestCgFragmentProfile = CgShared::getInstance()->getLatestFragmentProfile();

	pProgramManager->loadProgramFromFile( "LINEARCOMBINATION_linearCombination", cgPrefix + "LinearCombination.cg",
		"linearCombination", latestCgFragmentProfile, NULL );

	s_bCgInitialized = true;
}

// static
LinearCombinationKernel* LinearCombinationKernel::create( QString args )
{
	if( !s_bCgInitialized )
	{
		initializeCg();
	}

	LinearCombinationKernel* pInstance = new LinearCombinationKernel( args );
	pInstance->initializeGL();
	return pInstance;	
}

// virtual
LinearCombinationKernel::~LinearCombinationKernel()
{
	deleteSharedParameters();
	deleteOutputTexture();
}

// virtual
bool LinearCombinationKernel::isInputComplete()
{
	return true;
}

// virtual
void LinearCombinationKernel::makeDirty( QString inputPortName )
{
	m_bReallocationNeeded = true;
	m_pOutputTextureOutputPort->makeDirty();
}

// virtual
void LinearCombinationKernel::compute( QString outputPortName )
{
	if( m_bReallocationNeeded )
	{
		reallocate();
	}

	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pOutputTexture );	

	// setup inputs
	for( int i = 0; i < m_nInputTextures; ++i )
	{
		GLTextureRectangle* pInput = m_qvInputTextures.at( i );
		GLuint texId = pInput->getTextureId();

		CGparameter inputSamplerParameter = cgGetArrayParameter( m_cgp_shared_inputSamplersArray, i );
		cgGLSetTextureParameter( inputSamplerParameter, texId );

		float scales[4];
		scales[0] = m_qvScaleXs.at( i );
		scales[1] = m_qvScaleYs.at( i );
		scales[2] = m_qvScaleZs.at( i );
		scales[3] = m_qvScaleWs.at( i );

		CGparameter scaleParamter = cgGetArrayParameter( m_cgp_shared_f4_scalesArray, i );
		cgGLSetParameter4f( scaleParamter,
			m_qvScaleXs.at( i ),
			m_qvScaleYs.at( i ),
			m_qvScaleZs.at( i ),
			m_qvScaleWs.at( i ) );
	}

	glClear( GL_COLOR_BUFFER_BIT );
	GLUtilities::setupOrthoCamera( m_iInputWidth, m_iInputHeight );
	m_pFragmentProgram->bind();
	GLUtilities::drawQuad( m_iInputWidth, m_iInputHeight );
}

//////////////////////////////////////////////////////////////////////////
// protected
//////////////////////////////////////////////////////////////////////////

// virtual
void LinearCombinationKernel::initializeGL()
{
	GPUKernel::initializeGL();
	initializeCgPrograms();
}

// virtual					
void LinearCombinationKernel::initializePorts()
{
	for( int i = 0; i < m_nInputTextures; ++i )
	{
		QString portName = QString( "inputTexture%1" ).arg( i );
		InputKernelPort* pInputPort = addInputPort( portName, KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
		m_qvInputTexturesInputPort.append( pInputPort );
	}

	m_pOutputTextureOutputPort = addOutputPort( "outputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

// static
bool LinearCombinationKernel::s_bCgInitialized = false;

LinearCombinationKernel::LinearCombinationKernel( QString args ) :

	GPUKernel( "LinearCombination" ),

	m_pFBO( GLShared::getInstance()->getSharedFramebufferObject() ),

	m_bReallocationNeeded( true ),

	m_pOutputTexture( NULL ),

	m_iInputWidth( -1 ), // invalid
	m_iInputHeight( -1 ) // invalid

{
	QStringList scaleTokens = args.split( " ", QString::SkipEmptyParts );
	int nScaleTokens = scaleTokens.size();
	if( nScaleTokens < 4 )
	{
		fprintf( stderr, "LinearCombinationKernel needs at least 4 arguments\n" );
		exit( -1 );
	}

	if( ( nScaleTokens % 4 ) != 0 )
	{
		fprintf( stderr, "LinearCombinationKernel the number of arguments to be a multiple of 4\n" );
		exit( -1 );
	}

	m_nInputTextures = nScaleTokens / 4;
	for( int i = 0; i < m_nInputTextures; ++i )
	{	
		m_qvScaleXs.append( parseFloatOrExit( scaleTokens[ 4 * i ] ) );
		m_qvScaleYs.append( parseFloatOrExit( scaleTokens[ 4 * i + 1 ] ) );
		m_qvScaleZs.append( parseFloatOrExit( scaleTokens[ 4 * i + 2 ] ) );
		m_qvScaleWs.append( parseFloatOrExit( scaleTokens[ 4 * i + 3 ] ) );
	}
}

void LinearCombinationKernel::deleteSharedParameters()
{
	if( m_cgp_shared_f4_scalesArray != NULL )
	{
		cgDisconnectParameter( m_cgp_F_f4_scalesArray );
		cgDestroyParameter( m_cgp_shared_f4_scalesArray );
		m_cgp_shared_f4_scalesArray = NULL;
	}

	if( m_cgp_shared_inputSamplersArray != NULL )
	{
		cgDisconnectParameter( m_cgp_F_inputSamplersArray );
		cgDestroyParameter( m_cgp_shared_inputSamplersArray );
		m_cgp_shared_inputSamplersArray = NULL;
	}
}

void LinearCombinationKernel::deleteOutputTexture()
{
	if( m_pOutputTexture != NULL )
	{
		delete m_pOutputTexture;
		m_pOutputTexture = NULL;
	}
}

// virtual
void LinearCombinationKernel::reallocate()
{
	GLTextureRectangle* pInputTexture0 = m_qvInputTexturesInputPort.at( 0 )->pullData().getGLTextureRectangleData();
	bool sizeChanged = false;
	if( m_iInputWidth != pInputTexture0->getWidth() )
	{
		m_iInputWidth = pInputTexture0->getWidth();
		sizeChanged = true;
	}
	if( m_iInputHeight != pInputTexture0->getHeight() )
	{
		m_iInputHeight = pInputTexture0->getHeight();
		sizeChanged = true;
	}

	m_qvInputTextures.clear();
	m_qvInputTextures.append( pInputTexture0 );
	for( int i = 1; i < m_nInputTextures; ++i )
	{
		GLTextureRectangle* pTexture = m_qvInputTexturesInputPort.at( i )->pullData().getGLTextureRectangleData();
		
		assert( m_iInputWidth == pTexture->getWidth() );
		assert( m_iInputHeight == pTexture->getHeight() );

		m_qvInputTextures.append( pTexture );
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

void LinearCombinationKernel::initializeCgPrograms()
{
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();

	m_pFragmentProgram = pProgramManager->getNamedProgram( "LINEARCOMBINATION_linearCombination" );	

	m_cgp_F_inputSamplersArray = m_pFragmentProgram->getNamedParameter( "inputSamplers" );
	m_cgp_F_f4_scalesArray = m_pFragmentProgram->getNamedParameter( "scales" );

	CGcontext cgContext = CgShared::getInstance()->getSharedCgContext();
	m_cgp_shared_inputSamplersArray = cgCreateParameterArray( cgContext, CG_SAMPLERRECT, m_nInputTextures );
	m_cgp_shared_f4_scalesArray = cgCreateParameterArray( cgContext, CG_FLOAT4, m_nInputTextures );

	cgConnectParameter( m_cgp_shared_inputSamplersArray, m_cgp_F_inputSamplersArray );
	cgConnectParameter( m_cgp_shared_f4_scalesArray, m_cgp_F_f4_scalesArray );

	m_pFragmentProgram->load();
}

float LinearCombinationKernel::parseFloatOrExit( QString stringToken )
{
	bool ok;
	float fValue = stringToken.toFloat( &ok );

	if( !ok )
	{
		fprintf( stderr, "LinearCombinationKernel args must all be floats\n" );
		exit( -1 );
	}
	return fValue;
}
