#include "PerPixelProcessorKernel.h"

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
PerPixelProcessorKernel* PerPixelProcessorKernel::create( QString args )
{
	int instanceId = PerPixelProcessorKernel::s_nInstances;	
	
	PerPixelProcessorKernel* pInstance = new PerPixelProcessorKernel( instanceId, args );
	pInstance->initializeGL();

	++( PerPixelProcessorKernel::s_nInstances );

	return pInstance;
}

// virtual
PerPixelProcessorKernel::~PerPixelProcessorKernel()
{	
	deleteOutputTexture();
	deleteSharedParameters();
	
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	pProgramManager->destroyNamedProgram( m_qsProgramKey );
}

// virtual
bool PerPixelProcessorKernel::isInputComplete()
{
	return true;
}

// virtual
void PerPixelProcessorKernel::makeDirty( QString inputPortName )
{
	// TODO: only reallocate when texture changes
	m_bReallocationNeeded = true;
	m_pOutputTextureOutputPort->makeDirty();
}

// virtual
void PerPixelProcessorKernel::compute( QString outputPortName )
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

		CGparameter scaleParameter = cgGetArrayParameter( m_cgp_shared_f4_scalesArray, i );
		cgGLSetParameter4f( scaleParameter,
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
void PerPixelProcessorKernel::initializeGL()
{
	GPUKernel::initializeGL();
	initializeCgPrograms();
}

// virtual					
void PerPixelProcessorKernel::initializePorts()
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
int PerPixelProcessorKernel::s_nInstances = 0;

PerPixelProcessorKernel::PerPixelProcessorKernel( int instanceId, QString args ) :

	GPUKernel( "PerPixelProcessor" ),

	m_iInstanceId( instanceId ),
	m_pFBO( GLShared::getInstance()->getSharedFramebufferObject() ),

	m_bReallocationNeeded( true ),

	m_pOutputTexture( NULL ),

	m_iInputWidth( -1 ), // invalid
	m_iInputHeight( -1 ) // invalid
{
	m_qsProgramKey = QString( "PerPixelProcessor_process_%1" ).arg( m_iInstanceId );

	// first split the args into processors and scales, split on ";"
	QStringList processorsAndScales = args.split( ";", QString::SkipEmptyParts );
	if( processorsAndScales.size() < 2 )
	{
		fprintf( stderr, "PerPixelProcessor requires at least one processor and one set of xyzw scales\n" );
		exit( -1 );
	}

	// parse out all the processors
	m_qvProcessorTypeNames = processorsAndScales[ 0 ].split( " ", QString::SkipEmptyParts );
	m_nProcessors = m_qvProcessorTypeNames.size();
	if( m_nProcessors < 1 )
	{
		fprintf( stderr, "PerPixelProcessor requires at least one processor\n" );
		exit( -1 );
	}

	// parse out all the scales
	QStringList scaleTokens = processorsAndScales[ 1 ].split( " ", QString::SkipEmptyParts );
	int nScaleTokens = scaleTokens.size();
	if( nScaleTokens < 4 )
	{
		fprintf( stderr, "PerPixelProcessor needs at least 4 scales\n" );
		exit( -1 );
	}

	if( ( nScaleTokens % 4 ) != 0 )
	{
		fprintf( stderr, "PerPixelProcessor the number of scales to be a multiple of 4\n" );
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

void PerPixelProcessorKernel::deleteSharedParameters()
{
	// delete each processor in the array
	for( int i = 0; i < m_nProcessors; ++i )
	{
		CGparameter interfaceParameter = cgGetArrayParameter( m_cgp_shared_processorsArray, i );
		cgDisconnectParameter( interfaceParameter );
		cgDestroyParameter( m_qvStructParameters.at( i ) );
	}

	// delete the array
	if( m_cgp_shared_processorsArray != NULL )
	{
		cgDisconnectParameter( m_cgp_F_processorsArray );
		cgDestroyParameter( m_cgp_shared_processorsArray );
		m_cgp_shared_processorsArray = NULL;
	}

	// delete the scales array
	if( m_cgp_shared_f4_scalesArray != NULL )
	{
		cgDisconnectParameter( m_cgp_F_f4_scalesArray );
		cgDestroyParameter( m_cgp_shared_f4_scalesArray );
		m_cgp_shared_f4_scalesArray = NULL;
	}

	// delete the sampler array
	if( m_cgp_shared_inputSamplersArray != NULL )
	{
		cgDisconnectParameter( m_cgp_F_inputSamplersArray );
		cgDestroyParameter( m_cgp_shared_inputSamplersArray );
		m_cgp_shared_inputSamplersArray = NULL;
	}
}

void PerPixelProcessorKernel::deleteOutputTexture()
{
	if( m_pOutputTexture != NULL )
	{
		delete m_pOutputTexture;
		m_pOutputTexture = NULL;
	}
}

// virtual
void PerPixelProcessorKernel::reallocate()
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

void PerPixelProcessorKernel::initializeCgPrograms()
{
	instantiateProgram();
	instantiateSamplersAndScales();
	instantiateProcessors();

	// load program
	m_pFragmentProgram->load();
}

void PerPixelProcessorKernel::instantiateProgram()
{
	QString cgPrefix = AppData::getInstance()->getCgPathPrefix();
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	CGprofile latestCgFragmentProfile = CgShared::getInstance()->getLatestFragmentProfile();	

	pProgramManager->loadProgramFromFile( m_qsProgramKey, cgPrefix + "PerPixelProcessor.cg",
		"process", latestCgFragmentProfile, NULL );
	m_pFragmentProgram = pProgramManager->getNamedProgram( m_qsProgramKey );	

	m_cgp_F_inputSamplersArray = m_pFragmentProgram->getNamedParameter( "inputSamplers" );
	m_cgp_F_f4_scalesArray = m_pFragmentProgram->getNamedParameter( "scales" );
	m_cgp_F_processorsArray = m_pFragmentProgram->getNamedParameter( "processors" );
}

void PerPixelProcessorKernel::instantiateSamplersAndScales()
{
	CGcontext cgContext = CgShared::getInstance()->getSharedCgContext();
	m_cgp_shared_inputSamplersArray = cgCreateParameterArray( cgContext, CG_SAMPLERRECT, m_nInputTextures );
	m_cgp_shared_f4_scalesArray = cgCreateParameterArray( cgContext, CG_FLOAT4, m_nInputTextures );

	cgConnectParameter( m_cgp_shared_inputSamplersArray, m_cgp_F_inputSamplersArray );
	cgConnectParameter( m_cgp_shared_f4_scalesArray, m_cgp_F_f4_scalesArray );
}

void PerPixelProcessorKernel::instantiateProcessors()
{
	CGcontext cgContext = CgShared::getInstance()->getSharedCgContext();
	CGprogram cgProgram = m_pFragmentProgram->getProgram();
	CGtype processorInterfaceType = cgGetNamedUserType( cgProgram, "PerPixelProcessor" );

	// make a shared array of interfaces
	m_cgp_shared_processorsArray = cgCreateParameterArray( cgContext, processorInterfaceType, m_nProcessors );

	// instantiate each element with a concrete processor struct by name
	for( int i = 0; i < m_nProcessors; ++i )
	{
		printf( "processor no. %d, type = %s\n", i, qPrintable( m_qvProcessorTypeNames.at( i ) ) );

		CGparameter currentInterfaceParameter = cgGetArrayParameter( m_cgp_shared_processorsArray, i );		
		CGtype structType = cgGetNamedUserType( cgProgram, qPrintable( m_qvProcessorTypeNames.at( i ) ) );
		CGparameter currentStructParameter = cgCreateParameter( cgContext, structType );		

		cgConnectParameter( currentStructParameter, currentInterfaceParameter );
		m_qvStructParameters.append( currentStructParameter );
	}

	// now connect the shared array to the formal parameters of the shader
	cgConnectParameter( m_cgp_shared_processorsArray, m_cgp_F_processorsArray );
}

float PerPixelProcessorKernel::parseFloatOrExit( QString stringToken )
{
	bool ok;
	float fValue = stringToken.toFloat( &ok );

	if( !ok )
	{
		fprintf( stderr, "PerPixelProcessor: args must all be floats\n" );
		exit( -1 );
	}
	return fValue;
}
