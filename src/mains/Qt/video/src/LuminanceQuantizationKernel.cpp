#include "LuminanceQuantizationKernel.h"

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
void LuminanceQuantizationKernel::initializeCg()
{
	QString cgPrefix = AppData::getInstance()->getCgPathPrefix();
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	CGprofile latestCgFragmentProfile = CgShared::getInstance()->getLatestFragmentProfile();

	pProgramManager->loadProgramFromFile( "LQ_quantizeLuminance", cgPrefix + "LuminanceQuantization.cg",
		"quantizeLuminance", latestCgFragmentProfile, NULL );

	s_bCgInitialized = true;
}

// static
LuminanceQuantizationKernel* LuminanceQuantizationKernel::create( QString args )
{
	if( !s_bCgInitialized )
	{
		initializeCg();
	}

	LuminanceQuantizationKernel* pInstance = new LuminanceQuantizationKernel;
	pInstance->initializeGL();
	return pInstance;
}

// virtual
LuminanceQuantizationKernel::~LuminanceQuantizationKernel()
{
	deleteOutputTexture();
}

// virtual
bool LuminanceQuantizationKernel::isInputComplete()
{
	return true;
}

// virtual
void LuminanceQuantizationKernel::makeDirty( QString inputPortName )
{
	if( inputPortName == "inputTexture" )
	{
		m_bReallocationNeeded = true;
	}

	m_pOutputTextureOutputPort->makeDirty();
}

// virtual
void LuminanceQuantizationKernel::compute( QString outputPortName )
{
	if( m_bReallocationNeeded )
	{
		reallocate();		
	}

	// retrieve inputs
	int nBins = m_pNumBinsInputPort->pullData().getIntData();
	float sharpnessSlope = m_pSharpnessSlopeInputPort->pullData().getFloatData();
	float sharpnessOffset = m_pSharpnessOffsetInputPort->pullData().getFloatData();

	// derivet some parameters
	const float luminanceMin = 0;
	const float luminanceMax = 100;
	const float range = luminanceMax - luminanceMin;
	float binSize = range / ( nBins - 1 );
	float binMin = luminanceMin - binSize / 2;	

	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pOutputTexture );	

	// setup input
	cgGLSetTextureParameter( m_cgp_QL_inputLabASampler,
		m_pInputTexture->getTextureId() );

	// setup uniforms
	cgGLSetParameter1f( m_cgp_QL_fNumBins, nBins );
	cgGLSetParameter1f( m_cgp_QL_fBinMin, binMin );
	cgGLSetParameter1f( m_cgp_QL_fBinSize, binSize );
	cgGLSetParameter2f( m_cgp_QL_f2SharpnessAB,
		sharpnessSlope, sharpnessOffset );

	GLUtilities::setupOrthoCamera( m_iInputWidth, m_iInputHeight );
	m_pQuantizeLuminanceFragmentProgram->bind();
	GLUtilities::drawQuad( m_iInputWidth, m_iInputHeight );
}

//////////////////////////////////////////////////////////////////////////
// protected
//////////////////////////////////////////////////////////////////////////

// virtual
void LuminanceQuantizationKernel::initializeGL()
{
	GPUKernel::initializeGL();
	initializeCgPrograms();
}

// virtual
void LuminanceQuantizationKernel::initializePorts()
{
	// -- Input Ports --
	m_pInputTextureInputPort = addInputPort( "inputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );

	m_pNumBinsInputPort = addInputPort( "nBins", KERNEL_PORT_DATA_TYPE_INT );
	m_pNumBinsInputPort->setIntMinMaxDelta( 9, 1, 100, 1 ); // TODO: paper says 8 to 10 bins

	m_pSharpnessSlopeInputPort = addInputPort( "sharpnessSlope", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pSharpnessSlopeInputPort->setFloatMinMaxDelta( 11.f, 0.01f, 100.f, 1.f );

	m_pSharpnessOffsetInputPort = addInputPort( "sharpnessOffset", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pSharpnessOffsetInputPort->setFloatMinMaxDelta( 3.f, 0.01f, 100.f, 1.f );

	// -- Output Ports --
	m_pOutputTextureOutputPort = addOutputPort( "outputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

// static
bool LuminanceQuantizationKernel::s_bCgInitialized = false;

LuminanceQuantizationKernel::LuminanceQuantizationKernel() :

	GPUKernel( "LuminanceQuantization" ),

	m_pFBO( GLShared::getInstance()->getSharedFramebufferObject() ),

	m_bReallocationNeeded( true ),

	m_iInputWidth( -1 ), // invalid
	m_iInputHeight( -1 ), // invalid

	m_pOutputTexture( NULL )
{

}

void LuminanceQuantizationKernel::deleteOutputTexture()
{
	if( m_pOutputTexture != NULL )
	{
		delete m_pOutputTexture;
		m_pOutputTexture = NULL;
	}
}

// virtual
void LuminanceQuantizationKernel::reallocate()
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
		m_pOutputTexture = GLTextureRectangle::createFloat4Texture( m_iInputWidth, m_iInputHeight, 16 );
		m_pOutputTextureOutputPort->pushData( KernelPortData( m_pOutputTexture ) );
	}

	m_bReallocationNeeded = false;
}

void LuminanceQuantizationKernel::initializeCgPrograms()
{
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();

	m_pQuantizeLuminanceFragmentProgram = pProgramManager->getNamedProgram( "LQ_quantizeLuminance" );

	m_cgp_QL_inputLabASampler = m_pQuantizeLuminanceFragmentProgram->getNamedParameter( "inputLabASampler" );
	m_cgp_QL_fNumBins = m_pQuantizeLuminanceFragmentProgram->getNamedParameter( "nBins" );
	m_cgp_QL_fBinMin = m_pQuantizeLuminanceFragmentProgram->getNamedParameter( "binMin" );
	m_cgp_QL_fBinSize = m_pQuantizeLuminanceFragmentProgram->getNamedParameter( "binSize" );
	m_cgp_QL_f2SharpnessAB = m_pQuantizeLuminanceFragmentProgram->getNamedParameter( "sharpnessAB" );
}
