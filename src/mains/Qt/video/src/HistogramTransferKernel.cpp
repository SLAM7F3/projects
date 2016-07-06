#include "HistogramTransferKernel.h"

#include <cassert>
#include <Cg/CgProgramManager.h>
#include <Cg/CgProgramWrapper.h>
#include <Cg/CgShared.h>
#include <Cg/CgUtilities.h>
#include <GL/GLFramebufferObject.h>
#include <GL/GLShared.h>
#include <GL/GLTextureRectangle.h>
#include <GL/GLUtilities.h>
#include <io/PortableFloatMapIO.h>
#include <QString>
#include <QStringList>

#include "AppData.h"

//////////////////////////////////////////////////////////////////////////
// Public
//////////////////////////////////////////////////////////////////////////

// static
void HistogramTransferKernel::initializeCg()
{
	QString cgPrefix = AppData::getInstance()->getCgPathPrefix();
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	CGprofile latestCgFragmentProfile = CgShared::getInstance()->getLatestFragmentProfile();

	pProgramManager->loadProgramFromFile( "HISTOGRAMTRANSFER_transferHistogramStandard", cgPrefix + "Histogram.cg",
		"transferHistogramStandard", latestCgFragmentProfile, NULL );

	pProgramManager->loadProgramFromFile( "HISTOGRAMTRANSFER_transferHistogramAbsoluteValue", cgPrefix + "Histogram.cg",
		"transferHistogramAbsoluteValue", latestCgFragmentProfile, NULL );

	s_bCgInitialized = true;
}

// static
HistogramTransferKernel* HistogramTransferKernel::create( QString args )
{
	if( !s_bCgInitialized )
	{
		initializeCg();
	}

	HistogramTransferKernel* pInstance = new HistogramTransferKernel( args );
	pInstance->initializeGL();
	return pInstance;
}

// virtual
HistogramTransferKernel::~HistogramTransferKernel()
{
	// TODO: delete PFM
	deleteOutputTexture();
}

// virtual
bool HistogramTransferKernel::isInputComplete()
{
	return true;
}

// virtual
void HistogramTransferKernel::makeDirty( QString inputPortName )
{
	if( inputPortName == "nBins" )
	{
		m_bReallocationNeeded = true;
	}
	m_pOutputPort->makeDirty();
}

// virtual
void HistogramTransferKernel::compute( QString outputPortName )
{
	if( m_bReallocationNeeded )
	{
		reallocate();		
	}

	// retrieve data
	m_pInputCDF = m_pInputCDFInputPort->pullData().getGLTextureRectangleData();
	int inputCDFNumBins = m_pInputCDF->getWidth();

	// setup outputs
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pOutputTexture );

	// setup inputs
	cgGLSetTextureParameter( m_cgp_F_inputLabASampler, m_pInputTexture->getTextureId() );
	m_pInputCDF->setFilterMode( GLTexture::FILTER_MODE_LINEAR, GLTexture::FILTER_MODE_LINEAR );
	cgGLSetTextureParameter( m_cgp_F_inputCDFSampler, m_pInputCDF->getTextureId() );
	m_pModelInverseCDF->setFilterMode( GLTexture::FILTER_MODE_LINEAR, GLTexture::FILTER_MODE_LINEAR );
	cgGLSetTextureParameter( m_cgp_F_modelInverseCDFSampler, m_pModelInverseCDF->getTextureId() );

	// setup uniform
	cgGLSetParameter2f( m_cgp_F_f2InputCDFNumBinsModelInverseCDFNumBins,
		inputCDFNumBins, m_iModelInverseCDFNumBins );

	glClear( GL_COLOR_BUFFER_BIT );
	GLUtilities::setupOrthoCamera( m_iInputWidth, m_iInputHeight );	
	m_pFragmentProgram->bind();
	GLUtilities::drawQuad( m_iInputWidth, m_iInputHeight );

	// reset state
	m_pModelInverseCDF->setFilterMode( GLTexture::FILTER_MODE_NEAREST, GLTexture::FILTER_MODE_NEAREST );
	m_pInputCDF->setFilterMode( GLTexture::FILTER_MODE_NEAREST, GLTexture::FILTER_MODE_NEAREST );
}

//////////////////////////////////////////////////////////////////////////
// protected
//////////////////////////////////////////////////////////////////////////

// virtual
void HistogramTransferKernel::initializeGL()
{
	GPUKernel::initializeGL();
	initializeCgPrograms();

	int nBits = AppData::getInstance()->getTextureNumBits();

	m_pModelInverseCDF = GLTextureRectangle::createFloat1Texture( m_iModelInverseCDFNumBins, 1, nBits,
		m_afModelInverseCDFPixels );
}

// virtual					
void HistogramTransferKernel::initializePorts()
{
	// input ports
	m_pInputTextureInputPort = addInputPort( "inputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
	m_pInputCDFInputPort = addInputPort( "inputCDF", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );

	// output ports
	m_pOutputPort = addOutputPort( "outputTexture",	KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

// static
bool HistogramTransferKernel::s_bCgInitialized = false;

HistogramTransferKernel::HistogramTransferKernel( QString args ) :

	GPUKernel( "HistogramTransfer" ),
	m_pFBO( GLShared::getInstance()->getSharedFramebufferObject() ),

	m_bReallocationNeeded( true ),

	m_iInputWidth( -1 ), // invalid
	m_iInputHeight( -1 ), // invalid

	// outputs
	m_pOutputTexture( NULL )
{
	QStringList argumentList = args.split( " ", QString::SkipEmptyParts );
	if( argumentList.size() != 2 )
	{
		fprintf( stderr, "HistogramTransfer must have exactly 2 arguments\n" );
		exit( -1 );
	}

	QString arg0 = argumentList.at( 0 );
	if( arg0 == "standard" )
	{
		m_bIsAbsoluteValue = false;
	}
	else if( arg0 == "absolutevalue" )
	{
		m_bIsAbsoluteValue = true;
	}
	else
	{
		fprintf( stderr, "HistogramTransfer: arg0 must be \"standard\" or \"absolutevalue\"\n" );
		exit( -1 ); // exit, not assert, too dangerous
	}

	QString arg1 = argumentList.at( 1 );
	int height;
	int nComponents;
	float scale;

	bool succeeded = PortableFloatMapIO::read( arg1, true, &m_afModelInverseCDFPixels,
		&m_iModelInverseCDFNumBins, &height,
		&nComponents, &scale );	
	if( !succeeded )
	{
		fprintf( stderr, "HistogramTransfer: error loading inverse cdf pfm file: %s\n", qPrintable( arg1 ) );
		exit( -1 );
	}
}

void HistogramTransferKernel::deleteOutputTexture()
{
	if( m_pOutputTexture != NULL )
	{
		delete m_pOutputTexture;
		m_pOutputTexture = NULL;
	}
}

void HistogramTransferKernel::reallocate()
{
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
		m_pOutputPort->pushData( KernelPortData( m_pOutputTexture ) );
	}

	m_bReallocationNeeded = false;
}

void HistogramTransferKernel::initializeCgPrograms()
{
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	if( m_bIsAbsoluteValue )
	{
		m_pFragmentProgram = pProgramManager->getNamedProgram( "HISTOGRAMTRANSFER_transferHistogramAbsoluteValue" );
	}
	else
	{
		m_pFragmentProgram = pProgramManager->getNamedProgram( "HISTOGRAMTRANSFER_transferHistogramStandard" );
	}

	m_cgp_F_inputLabASampler = m_pFragmentProgram->getNamedParameter( "inputLabASampler" );
	m_cgp_F_inputCDFSampler = m_pFragmentProgram->getNamedParameter( "inputCDFSampler" );
	m_cgp_F_modelInverseCDFSampler = m_pFragmentProgram->getNamedParameter( "modelInverseCDFSampler" );
	m_cgp_F_f2InputCDFNumBinsModelInverseCDFNumBins = m_pFragmentProgram->getNamedParameter( "inputCDFNumBinsModelInverseCDFNumBins" );
}
