#include "StylizeKernel.h"

#include <cassert>
#include <math/MathUtils.h>

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
void StylizeKernel::initializeCg()
{
	QString cgPrefix = AppData::getInstance()->getCgPathPrefix();
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	CGprofile latestCgFragmentProfile = CgShared::getInstance()->getLatestFragmentProfile();

	pProgramManager->loadProgramFromFile( "STYLIZE_decomposeBaseDetailAndApplyCurve", cgPrefix + "Stylize.cg",
		"decomposeBaseDetailAndApplyCurve", latestCgFragmentProfile, NULL );

	s_bCgInitialized = true;
}

// static
StylizeKernel* StylizeKernel::create( QString args )
{
	if( !s_bCgInitialized )
	{
		initializeCg();
	}

	StylizeKernel* pInstance = new StylizeKernel;
	pInstance->initializeGL();
	return pInstance;
}

// virtual
StylizeKernel::~StylizeKernel()
{
	deleteRemappingCurveData();
	deleteOutputTexture();
}

// virtual
bool StylizeKernel::isInputComplete()
{
	return true;
}

// virtual
void StylizeKernel::makeDirty( QString inputPortName )
{
	if( inputPortName == "inputRGBTexture" ||
		inputPortName == "bfDenoisedLabTexture" ||
		inputPortName == "bfBaseLabTexture" ||
		inputPortName == "baseRemappingCurve" ||
		inputPortName == "detailRemappingCurve" )
	{
		m_bReallocationNeeded = true;
	}

	if( inputPortName == "baseRemappingCurve" )
	{
		m_bBaseCurveTextureUpdateNeeded = true;
	}
	else if( inputPortName == "detailRemappingCurve" )
	{
		m_bDetailCurveTextureUpdatedNeeded = true;
	}
	
	m_pOutputTextureOutputPort->makeDirty();
}

// virtual
void StylizeKernel::compute( QString outputPortName )
{
	if( m_bReallocationNeeded )
	{
		reallocate();		
	}

	// retrieve inputs
	m_fLambda = m_pLambdaDetailInputPort->pullData().getFloatData();

	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pOutputTexture );

	// setup inputs
	cgGLSetTextureParameter( m_cgp_inputRGBASampler,
		m_pInputRGBTexture->getTextureId() );
	cgGLSetTextureParameter( m_cgp_bfDenoisedLabASampler,
		m_pBFDenoisedLabTexture->getTextureId() );
	cgGLSetTextureParameter( m_cgp_bfBaseLabASampler,
		m_pBFBaseLabTexture->getTextureId() );
	cgGLSetTextureParameter( m_cgp_remappingCurveSampler,
		m_pRemappingCurveTexture->getTextureId() );	

	// setup uniforms	
	cgGLSetParameter1f( m_cgp_fRemappingCurveWidth, m_pRemappingCurveTexture->getWidth() );
	cgGLSetParameter1f( m_cgp_fLambdaDetail, m_fLambda );
	cgGLSetParameter1f( m_cgp_fDetailMax, 0.2f ); // HACK

	glClear( GL_COLOR_BUFFER_BIT );
	GLUtilities::setupOrthoCamera( m_iWidth, m_iHeight );
	m_pFragmentProgram->bind();
	GLUtilities::drawQuad( m_iWidth, m_iHeight );
}

//////////////////////////////////////////////////////////////////////////
// protected
//////////////////////////////////////////////////////////////////////////

// virtual
void StylizeKernel::initializeGL()
{
	GPUKernel::initializeGL();
	initializeCgPrograms();
}

// virtual
void StylizeKernel::initializePorts()
{
	// input ports
	m_pInputRGBTextureInputPort = addInputPort( "inputRGBTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
	m_pBFDenoisedTextureInputPort = addInputPort( "bfDenoisedLabTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
	m_pBFBaseTextureInputPort = addInputPort( "bfBaseLabTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );

	m_pLambdaDetailInputPort = addInputPort( "lambdaDetail", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pLambdaDetailInputPort->setFloatMinMaxDelta( 1.f, 0.f, 20.f, 0.1f );

	m_pBaseRemappingCurveInputPort = addInputPort
	(
		"baseRemappingCurve",
		KERNEL_PORT_DATA_TYPE_FLOAT_ARRAY
	);

	m_pDetailRemappingCurveInputPort = addInputPort
	(
		"detailRemappingCurve",
		KERNEL_PORT_DATA_TYPE_FLOAT_ARRAY
	);

	// output ports
	m_pOutputTextureOutputPort = addOutputPort( "outputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

// static
bool StylizeKernel::s_bCgInitialized = false;

StylizeKernel::StylizeKernel() :

	GPUKernel( "Stylize" ),

	m_pFBO( GLShared::getInstance()->getSharedFramebufferObject() ),

	m_bReallocationNeeded( true ),

	m_iWidth( -1 ), // invalid
	m_iHeight( -1 ), // invalid
	m_pInputRGBTexture( NULL ),
	m_pBFDenoisedLabTexture( NULL ),
	m_pBFBaseLabTexture( NULL ),

	m_iRemappingCurveLength( -1 ), // invalid
	m_pRemappingCurveTexture( NULL ),
	m_aubScaledRemappingCurve( NULL ),
	
	m_fLambda( -1 ),

	m_pOutputTexture( NULL ),

	m_bBaseCurveTextureUpdateNeeded( true ),
	m_bDetailCurveTextureUpdatedNeeded( true )
{

}

void StylizeKernel::reallocate()
{
	// output texture size
	m_pInputRGBTexture = m_pInputRGBTextureInputPort->pullData().getGLTextureRectangleData();
	m_pBFDenoisedLabTexture = m_pBFDenoisedTextureInputPort->pullData().getGLTextureRectangleData();
	m_pBFBaseLabTexture = m_pBFBaseTextureInputPort->pullData().getGLTextureRectangleData();

	int inputTextureWidth = m_pInputRGBTexture->getWidth();
	int inputTextureHeight = m_pInputRGBTexture->getHeight();

	assert( inputTextureWidth == m_pBFDenoisedLabTexture->getWidth() );
	assert( inputTextureHeight == m_pBFDenoisedLabTexture->getHeight() );
	assert( inputTextureWidth == m_pBFBaseLabTexture->getWidth() );
	assert( inputTextureHeight == m_pBFBaseLabTexture->getHeight() );

	bool sizeChanged = false;
	if( m_iWidth != inputTextureWidth )
	{
		m_iWidth = inputTextureWidth;
		sizeChanged = true;
	}
	if( m_iHeight != inputTextureHeight )
	{
		m_iHeight = inputTextureHeight;
		sizeChanged = true;
	}

	if( sizeChanged )
	{
		deleteOutputTexture();

		m_pOutputTexture = GLTextureRectangle::createFloat4Texture( m_iWidth, m_iHeight, 16 );
		m_pOutputTextureOutputPort->pushData( KernelPortData( m_pOutputTexture ) );
	}

	m_qvBaseRemappingCurve = m_pBaseRemappingCurveInputPort->pullData().getFloatArrayData();
	int baseRemappingCurveLength = m_qvBaseRemappingCurve.size();

	m_qvDetailRemappingCurve = m_pDetailRemappingCurveInputPort->pullData().getFloatArrayData();
	assert( baseRemappingCurveLength == m_qvDetailRemappingCurve.size() );

	if( m_iRemappingCurveLength != baseRemappingCurveLength )
	{
		deleteRemappingCurveData();
		m_iRemappingCurveLength = baseRemappingCurveLength;

		m_aubScaledRemappingCurve = new ubyte[ 2 * m_iRemappingCurveLength ];
		rescaleRemappingCurves();
		m_pRemappingCurveTexture = GLTextureRectangle::createUnsignedByte1Texture( m_iRemappingCurveLength, 2, m_aubScaledRemappingCurve );
		m_pRemappingCurveTexture->setFilterMode( GLTexture::FILTER_MODE_LINEAR, GLTexture::FILTER_MODE_LINEAR );
	}
	else if( m_bBaseCurveTextureUpdateNeeded )
	{
		rescaleRemappingCurves();
		m_pRemappingCurveTexture->setUnsignedByte1Data( m_aubScaledRemappingCurve, 0, 0, m_iRemappingCurveLength, 1 );

		m_bBaseCurveTextureUpdateNeeded = false;
	}
	else if( m_bDetailCurveTextureUpdatedNeeded )
	{
		rescaleRemappingCurves();
		m_pRemappingCurveTexture->setUnsignedByte1Data( &( m_aubScaledRemappingCurve[ m_iRemappingCurveLength ] ),
			0, 1, m_iRemappingCurveLength, 1 );

		m_bDetailCurveTextureUpdatedNeeded = false;
	}

	m_bReallocationNeeded = false;
}

void StylizeKernel::deleteOutputTexture()
{
	if( m_pOutputTexture != NULL )
	{
		delete m_pOutputTexture;
		m_pOutputTexture = NULL;
	}
}

void StylizeKernel::initializeCgPrograms()
{
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();

	m_pFragmentProgram = pProgramManager->getNamedProgram( "STYLIZE_decomposeBaseDetailAndApplyCurve" );

	m_cgp_inputRGBASampler = m_pFragmentProgram->getNamedParameter( "inputRGBASampler" );
	m_cgp_bfDenoisedLabASampler = m_pFragmentProgram->getNamedParameter( "bfDenoisedLabASampler" );
	m_cgp_bfBaseLabASampler = m_pFragmentProgram->getNamedParameter( "bfBaseLabASampler" );
	m_cgp_remappingCurveSampler = m_pFragmentProgram->getNamedParameter( "remappingCurveSampler" );
	m_cgp_fRemappingCurveWidth = m_pFragmentProgram->getNamedParameter( "remappingCurveWidth" );
	m_cgp_fLambdaDetail = m_pFragmentProgram->getNamedParameter( "lambdaDetail" );
	m_cgp_fDetailMax = m_pFragmentProgram->getNamedParameter( "detailMax" );
}

void StylizeKernel::rescaleRemappingCurves()
{
	for( int i = 0; i < m_iRemappingCurveLength; ++i )
	{
		m_aubScaledRemappingCurve[i] = ( ubyte )
		(
			255.0f * MathUtils::clampToRangeFloat
			(
				m_qvBaseRemappingCurve.at( i ),
				0.0f, 1.0f
			)
		);
	}

	for( int i = 0; i < m_iRemappingCurveLength; ++i )
	{
		m_aubScaledRemappingCurve[ m_iRemappingCurveLength + i ] = ( ubyte )
		(
			255.0f * MathUtils::clampToRangeFloat
			(
				m_qvDetailRemappingCurve.at( i ),
				0.0f, 1.0f
			)
		);
	}
}

void StylizeKernel::deleteRemappingCurveData()
{
	if( m_pRemappingCurveTexture != NULL )
	{
		delete m_pRemappingCurveTexture;
		m_pRemappingCurveTexture = NULL;
	}

	if( m_aubScaledRemappingCurve != NULL )
	{
		delete m_aubScaledRemappingCurve;
		m_aubScaledRemappingCurve = NULL;
	}
}

#if 0
void StylizeKernel::initializeCgPasses()
{
	CgEffectManager* pEffectManager = CgEffectManager::getInstance();
	m_cgEffect = pEffectManager->getNamedEffect( "Stylize" );
	CGtechnique technique = cgGetNamedTechnique
	(
		m_cgEffect,
		"Stylize"
	);
	CgUtilities::validateTechnique( technique );

	m_cgPass = cgGetFirstPass( technique );

#if _DEBUG
	assert( m_cgPass != NULL );
#endif
}

void StylizeKernel::initializeCgParameters()
{
	CGparameter parameter;

	// samplers
	parameter = cgGetNamedEffectParameter( m_cgEffect, "g_samplerInputRGB" );
#if _DEBUG
	assert( parameter != NULL );
#endif
	m_cgParameterSamplerInputRGB = parameter;

	parameter = cgGetNamedEffectParameter( m_cgEffect, "g_samplerBFDenoisedLab" );
#if _DEBUG
	assert( parameter != NULL );
#endif
	m_cgParameterSamplerBFDenoisedLab = parameter;

	parameter = cgGetNamedEffectParameter( m_cgEffect, "g_samplerBFBaseLab" );
#if _DEBUG
	assert( parameter != NULL );
#endif
	m_cgParameterSamplerBFBaseLab = parameter;

	parameter = cgGetNamedEffectParameter( m_cgEffect, "g_samplerRemappingCurve" );
#if _DEBUG
	assert( parameter != NULL );
#endif
	m_cgParameterSamplerRemappingCurve = parameter;

	// parameters
	parameter = cgGetNamedEffectParameter( m_cgEffect, "g_fLambdaDetail" );
#if _DEBUG
	assert( parameter != NULL );
#endif
	m_cgParameterLambdaDetail = parameter;
}
#endif
