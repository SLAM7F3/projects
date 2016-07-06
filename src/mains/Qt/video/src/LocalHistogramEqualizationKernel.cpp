#include "LocalHistogramEqualizationKernel.h"

#include <cassert>
#include <Cg/CgProgramManager.h>
#include <Cg/CgProgramWrapper.h>
#include <Cg/CgShared.h>
#include <Cg/CgUtilities.h>
#include <GL/GLVertexBufferObject.h>
#include <GL/GLFramebufferObject.h>
#include <GL/GLShared.h>
#include <GL/GLTextureRectangle.h>
#include <GL/GLUtilities.h>

#include "AppData.h"

//////////////////////////////////////////////////////////////////////////
// public
//////////////////////////////////////////////////////////////////////////

// static
void LocalHistogramEqualizationKernel::initializeCg()
{
	QString cgPrefix = AppData::getInstance()->getCgPathPrefix();
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	CGprofile latestCgVertexProfile = CgShared::getInstance()->getLatestVertexProfile();
	CGprofile latestCgFragmentProfile = CgShared::getInstance()->getLatestFragmentProfile();

	pProgramManager->loadProgramFromFile( "LHE_buildPDFGridVertex", cgPrefix + "LocalHistogramEqualization.cg",
		"buildPDFGridVertex", latestCgVertexProfile, NULL );

	pProgramManager->loadProgramFromFile( "LHE_buildPDFGridFragment", cgPrefix + "LocalHistogramEqualization.cg",
		"buildPDFGridFragment", latestCgFragmentProfile, NULL );

	pProgramManager->loadProgramFromFile( "LHE_buildCDFGrid", cgPrefix + "LocalHistogramEqualization.cg",
		"buildCDFGrid", latestCgFragmentProfile, NULL );

	pProgramManager->loadProgramFromFile( "LHE_normalizeCDFGrid", cgPrefix + "LocalHistogramEqualization.cg",
		"normalizeCDFGrid", latestCgFragmentProfile, NULL );

	pProgramManager->loadProgramFromFile( "LHE_sliceGrid", cgPrefix + "LocalHistogramEqualization.cg",
		"sliceGrid", latestCgFragmentProfile, NULL );

	s_bCgInitialized = true;
}

// static
LocalHistogramEqualizationKernel* LocalHistogramEqualizationKernel::create( QString args )
{
	if( !s_bCgInitialized )
	{
		initializeCg();
	}

	LocalHistogramEqualizationKernel* pInstance = new LocalHistogramEqualizationKernel;
	pInstance->initializeGL();
	return pInstance;	
}

// virtual
LocalHistogramEqualizationKernel::~LocalHistogramEqualizationKernel()
{
	cleanup();
}

// virtual
bool LocalHistogramEqualizationKernel::isInputComplete()
{
	return true;
}

// virtual
void LocalHistogramEqualizationKernel::makeDirty( QString inputPortName )
{
	if( inputPortName == "inputTexture" ||
		inputPortName == "inputMin" ||
		inputPortName == "inputMax" )
	{
		m_bReallocationNeeded = true;
	}

	m_pOutputTextureOutputPort->makeDirty();
}

// virtual
void LocalHistogramEqualizationKernel::compute( QString outputPortName )
{
	if( m_bReallocationNeeded )
	{
		reallocate();
	}

	// retrieve inputs
	m_fSigmaSpatial = m_pSigmaSpatialInputPort->pullData().getFloatData();
	m_fSigmaRange = m_pSigmaRangeInputPort->pullData().getFloatData();
	m_fInputMin = m_fInputMinInputPort->pullData().getFloatData();
	m_fInputMax = m_fInputMaxInputPort->pullData().getFloatData();
	m_fInputDelta = m_fInputMax - m_fInputMin;

	m_iGridWidth = static_cast< int >( ( m_iInputWidth - 1 ) / m_fSigmaSpatial ) + 1 + 2 * m_iPadding;
	m_iGridHeight = static_cast< int >( ( m_iInputHeight - 1 ) / m_fSigmaSpatial ) + 1 + 2 * m_iPadding;
	m_iGridDepth = static_cast< int >( m_fInputDelta / m_fSigmaRange ) + 1 + 2 * m_iPadding;

	buildPDFGridPass();
	
	GLUtilities::setupOrthoCamera( m_iGridTextureWidth, m_iGridTextureHeight );
	buildCDFGridPass();
	normalizeCDFGridPass();	

	GLUtilities::setupOrthoCamera( m_iInputWidth, m_iInputHeight );
	sliceGridPass();
}

//////////////////////////////////////////////////////////////////////////
// protected
//////////////////////////////////////////////////////////////////////////

// virtual
void LocalHistogramEqualizationKernel::initializeGL()
{
	GPUKernel::initializeGL();
	initializeCgPrograms();

	// for video
	// TODO: make this an arg
	int maxTextureSize = GLTexture::getMaxTextureSize();
	m_iGridTextureWidth = maxTextureSize;
	m_iGridTextureHeight = 256;

	// TODO: shared on nComponents and nBits, maybe format?
	QVector< GLTextureRectangle* > tempGrids =
		GLShared::getInstance()->getSharedTexture
		(
			m_iGridTextureWidth, m_iGridTextureHeight, 2
		);

	m_apGrids[ 0 ] = tempGrids[ 0 ];
	m_apGrids[ 1 ] = tempGrids[ 1 ];
}

// virtual
void LocalHistogramEqualizationKernel::initializePorts()
{
	// input ports
	m_pInputTextureInputPort = addInputPort( "inputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );

	m_pSigmaSpatialInputPort = addInputPort( "sigmaSpatial", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pSigmaSpatialInputPort->setFloatMinMaxDelta( 128, 75, 512, 1 );

	m_pSigmaRangeInputPort = addInputPort( "sigmaRange", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pSigmaRangeInputPort->setFloatMinMaxDelta( 0.004f, 0.004f, 1.f, 0.001f );

	m_fInputMinInputPort = addInputPort( "inputMin", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_fInputMaxInputPort = addInputPort( "inputMax", KERNEL_PORT_DATA_TYPE_FLOAT );

	// output ports
	m_pOutputTextureOutputPort = addOutputPort( "outputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

// static
bool LocalHistogramEqualizationKernel::s_bCgInitialized = false;

LocalHistogramEqualizationKernel::LocalHistogramEqualizationKernel() :

	GPUKernel( "LocalHistogramEqualization" ),

	m_pFBO( GLShared::getInstance()->getSharedFramebufferObject() ),

	m_bReallocationNeeded( true ),

	m_pInputTexture( NULL ),
	m_pOutputTexture( NULL ),

	m_pXYCoordinateVBO( NULL ),	

	m_iInputWidth( -1 ), // invalid
	m_iInputHeight( -1 ), // invalid

	m_iPadding( 1 )
{
	m_apGrids[0] = NULL;
	m_apGrids[1] = NULL;
}

void LocalHistogramEqualizationKernel::cleanup()
{
	if( m_pOutputTexture != NULL )
	{
		delete m_pOutputTexture;
		m_pOutputTexture = NULL;
	}

	if( m_pXYCoordinateVBO != NULL )
	{
		delete m_pXYCoordinateVBO;
		m_pXYCoordinateVBO = NULL;
	}

	m_iOutputGridIndex = 0;
}

// virtual
void LocalHistogramEqualizationKernel::reallocate()
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

	m_nPixels = m_iInputWidth * m_iInputHeight;

	if( sizeChanged )
	{
		cleanup();

		int nBits = AppData::getInstance()->getTextureNumBits();

		GLshort* xyCoords = new GLshort[ 2 * m_iInputWidth * m_iInputHeight ];
		int index = 0;

		for( int y = 0; y < m_iInputHeight; ++y )
		{
			for( int x = 0; x < m_iInputWidth; ++x )
			{
				xyCoords[ index ] = x;
				xyCoords[ index + 1 ] = y;

				index += 2;
			}
		}

		m_pXYCoordinateVBO = GLVertexBufferObject::fromShortArray
		(
			xyCoords,
			2 * m_nPixels,
			m_nPixels
		);

		delete[] xyCoords;

		m_pOutputTexture = GLTextureRectangle::createFloat4Texture( m_iInputWidth, m_iInputHeight, nBits );
		m_pOutputTextureOutputPort->pushData( KernelPortData( m_pOutputTexture ) );
	}

	m_bReallocationNeeded = false;
}

void LocalHistogramEqualizationKernel::initializeCgPrograms()
{
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();

	//////////////////////////////////////////////////////////////////////////

	// ---- Vertex ----

	m_pBuildPDFGridVertexProgram = pProgramManager->getNamedProgram( "LHE_buildPDFGridVertex" );

	m_cgp_BPGV_f44_mvp = m_pBuildPDFGridVertexProgram->getNamedParameter( "mvp" );
	m_cgp_BPGV_f3_rcpSigma = m_pBuildPDFGridVertexProgram->getNamedParameter( "rcpSigma" );
	m_cgp_BPGV_f3_gridSize = m_pBuildPDFGridVertexProgram->getNamedParameter( "gridSize" );
	m_cgp_BPGV_f_inputMin = m_pBuildPDFGridVertexProgram->getNamedParameter( "inputMin" );
	m_cgp_BPGV_inputSampler = m_pBuildPDFGridVertexProgram->getNamedParameter( "inputSampler" );

	// ---- Fragment ----

	m_pBuildPDFGridFragmentProgram = pProgramManager->getNamedProgram( "LHE_buildPDFGridFragment" );

	//////////////////////////////////////////////////////////////////////////

	m_pBuildCDFGridFragmentProgram = pProgramManager->getNamedProgram( "LHE_buildCDFGrid" );
	m_cgp_BCGF_pdfGridSampler = m_pBuildCDFGridFragmentProgram->getNamedParameter( "pdfGridSampler" );
	m_cgp_BCGF_fGridWidth = m_pBuildCDFGridFragmentProgram->getNamedParameter( "gridWidth" );

	//////////////////////////////////////////////////////////////////////////

	m_pNormalizeCDFGridFragmentProgram = pProgramManager->getNamedProgram( "LHE_normalizeCDFGrid" );
	m_cgp_NCGF_cdfGridSampler = m_pNormalizeCDFGridFragmentProgram->getNamedParameter( "cdfGridSampler" );
	m_cgp_NCGF_f3GridWidthHeightDepth = m_pNormalizeCDFGridFragmentProgram->getNamedParameter( "gridSize" );

	//////////////////////////////////////////////////////////////////////////

	m_pSliceGridFragmentProgram = pProgramManager->getNamedProgram( "LHE_sliceGrid" );

	m_cgp_SGF_inputSampler = m_pSliceGridFragmentProgram->getNamedParameter( "inputSampler" );
	m_cgp_SGF_equalizedGridSampler = m_pSliceGridFragmentProgram->getNamedParameter( "equalizedGridSampler" );
	m_cgp_SGF_f3_rcpSigma = m_pSliceGridFragmentProgram->getNamedParameter( "rcpSigma" );
	m_cgp_SGF_f3_gridSize = m_pSliceGridFragmentProgram->getNamedParameter( "gridSize" );
	m_cgp_SGF_f_inputMin = m_pSliceGridFragmentProgram->getNamedParameter( "inputMin" );
	m_cgp_SGF_f_inputDelta = m_pSliceGridFragmentProgram->getNamedParameter( "inputDelta" );
}

GLTextureRectangle* LocalHistogramEqualizationKernel::getInputGrid()
{
	return m_apGrids[ ( m_iOutputGridIndex + 1 ) % 2 ];
}

GLTextureRectangle* LocalHistogramEqualizationKernel::getOutputGrid()
{
	return m_apGrids[ m_iOutputGridIndex ];
}

void LocalHistogramEqualizationKernel::swapGrids()
{
	m_iOutputGridIndex = ( m_iOutputGridIndex + 1 ) % 2;
}

void LocalHistogramEqualizationKernel::buildPDFGridPass()
{
	// setup output
	cgGLEnableProfile( CgShared::getInstance()->getLatestVertexProfile() );
	glEnable( GL_BLEND );
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, getOutputGrid() );

	// setup inputs
	cgGLSetTextureParameter( m_cgp_BPGV_inputSampler,
		m_pInputTexture->getTextureId() );

	// set uniforms
	CgUtilities::setupOrthoCamera( m_iGridTextureWidth, m_iGridTextureHeight,
		m_cgp_BPGV_f44_mvp );
	cgGLSetParameter3f( m_cgp_BPGV_f3_rcpSigma,
		1.f / m_fSigmaSpatial, 1.f / m_fSigmaSpatial, 1.f / m_fSigmaRange );
	cgGLSetParameter3f( m_cgp_BPGV_f3_gridSize,
		m_iGridWidth, m_iGridHeight, m_iGridDepth );
	cgGLSetParameter1f( m_cgp_BPGV_f_inputMin, m_fInputMin );

	// render
	glClear( GL_COLOR_BUFFER_BIT );
	m_pBuildPDFGridVertexProgram->bind();
	m_pBuildPDFGridFragmentProgram->bind();
	m_pXYCoordinateVBO->bind( GLBufferObject::TARGET_ARRAY_BUFFER );
	glVertexPointer( 2, GL_SHORT, 0, NULL );
	glDrawArrays( GL_POINTS, 0, m_pXYCoordinateVBO->getNumElements() );
	GLBufferObject::unbind( GLBufferObject::TARGET_ARRAY_BUFFER );

	glDisable( GL_BLEND );
	cgGLDisableProfile( CgShared::getInstance()->getLatestVertexProfile() );		

	swapGrids();
}

void LocalHistogramEqualizationKernel::buildCDFGridPass()
{
	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, getOutputGrid() );

	// setup input
	cgGLSetTextureParameter( m_cgp_BCGF_pdfGridSampler,
		getInputGrid()->getTextureId() );

	// setup uniform
	cgGLSetParameter1f( m_cgp_BCGF_fGridWidth, m_iGridWidth );

	// render
	glClear( GL_COLOR_BUFFER_BIT );	
	m_pBuildCDFGridFragmentProgram->bind();
	GLUtilities::drawQuad( m_iGridWidth * m_iGridDepth, m_iGridHeight );	

	swapGrids();
}

void LocalHistogramEqualizationKernel::normalizeCDFGridPass()
{
	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, getOutputGrid() );

	// setup input
	cgGLSetTextureParameter( m_cgp_NCGF_cdfGridSampler,
		getInputGrid()->getTextureId() );	

	// setup uniform
	cgGLSetParameter3f( m_cgp_NCGF_f3GridWidthHeightDepth,
		m_iGridWidth, m_iGridHeight, m_iGridDepth );

	// render
	glClear( GL_COLOR_BUFFER_BIT );
	m_pNormalizeCDFGridFragmentProgram->bind();
	GLUtilities::drawQuad( m_iGridWidth * m_iGridDepth, m_iGridHeight );

	swapGrids();
}

void LocalHistogramEqualizationKernel::sliceGridPass()
{
	// retrieve inputs
	GLTextureRectangle* pEqualizedGrid = getInputGrid();

	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pOutputTexture );

	// setup input
	cgGLSetTextureParameter( m_cgp_SGF_inputSampler,
		m_pInputTexture->getTextureId() );
	pEqualizedGrid->setFilterMode( GLTexture::FILTER_MODE_LINEAR, GLTexture::FILTER_MODE_LINEAR );
	cgGLSetTextureParameter( m_cgp_SGF_equalizedGridSampler,
		pEqualizedGrid->getTextureId() );

	// setup uniforms
	cgGLSetParameter3f( m_cgp_SGF_f3_rcpSigma,
		1.f / m_fSigmaSpatial, 1.f / m_fSigmaSpatial, 1.f / m_fSigmaRange );
	cgGLSetParameter3f( m_cgp_SGF_f3_gridSize,
		m_iGridWidth, m_iGridHeight, m_iGridDepth );
	cgGLSetParameter1f( m_cgp_SGF_f_inputMin, m_fInputMin );
	cgGLSetParameter1f( m_cgp_SGF_f_inputDelta, m_fInputDelta );

	glClear( GL_COLOR_BUFFER_BIT );		
	m_pSliceGridFragmentProgram->bind();
	GLUtilities::drawQuad( m_iInputWidth, m_iInputHeight );

	// restore state
	pEqualizedGrid->setFilterMode( GLTexture::FILTER_MODE_NEAREST, GLTexture::FILTER_MODE_NEAREST );

}
