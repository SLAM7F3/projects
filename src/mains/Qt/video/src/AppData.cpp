#include "AppData.h"

#include <cassert>
#include <cstdio>
#include <Cg/CgShared.h>
#include <GL/GLFramebufferObject.h>
#include <GL/GLShared.h>
#include <GL/GLTextureRectangle.h>
#include <GL/GLTexture3D.h>
#include <GL/GLUtilities.h>
#include <math/Arithmetic.h>
#include <QFile>

#include "InputKernelPort.h"
#include "KernelGraph.h"
#include "KernelPortData.h"
#include "KernelPortDataType.h"
#include "OutputKernelPort.h"
#include "OutputWidget.h"

//////////////////////////////////////////////////////////////////////////
// Public
//////////////////////////////////////////////////////////////////////////

// static
AppData* AppData::getInstance()
{
	if( s_pSingleton == NULL )
	{
		s_pSingleton = new AppData;
	}
	return s_pSingleton;
}

void AppData::setCgPathPrefix( QString str )
{
	if( !( str.endsWith( "/" ) || str.endsWith( "\\" ) ) )
	{		
		str += "/";
	}

	m_qsCgPathPrefix = str;
}

QString AppData::getCgPathPrefix()
{
	return m_qsCgPathPrefix;	
}

void AppData::setTextureNumBits( int nBits )
{
	m_nBits = nBits;
}

int AppData::getTextureNumBits()
{
	return m_nBits;
}

void AppData::setOutputWidthHeight( int width, int height )
{
	m_iOutputWidth = width;
	m_iOutputHeight = height;
}

QSize AppData::getOutputWidthHeight()
{
	return QSize( m_iOutputWidth, m_iOutputHeight );
}

int AppData::getTextureNumComponents()
{
	return 1;
}

void AppData::initializeGL()
{
	glewInit();

	GLTextureRectangle::setEnabled( true );
	glBlendFunc( GL_ONE, GL_ONE );
	glPointSize( 1 );
	glLineWidth( 1 );
	glPixelStorei( GL_PACK_ALIGNMENT, 1 );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glEnableClientState( GL_VERTEX_ARRAY );

	// create FBO
	m_pFBO = GLShared::getInstance()->getSharedFramebufferObject();
	cgGLEnableProfile( CgShared::getInstance()->getLatestVertexProfile() );
	cgGLEnableProfile( CgShared::getInstance()->getLatestFragmentProfile() );
}

KernelGraph* AppData::getGraph()
{
	return m_pGraph;
}

void AppData::setGraph( KernelGraph* pGraph )
{
	m_pGraph = pGraph;
}

void AppData::setAutoDirtyKernelsAndPorts( QVector< QPair< QString, QString > > autoDirtyKernelsAndPorts )
{
	m_qvAutoDirtyKernelsAndPorts = autoDirtyKernelsAndPorts;
}

OutputWidget* AppData::getOutputWidget()
{
	return m_pOutputWidget;
}

void AppData::setOutputWidget( OutputWidget* pWidget )
{
	m_qvObservedPorts.clear();
	m_qvObservedPortsArrayIndex.clear();

	m_pOutputWidget = pWidget;

	int nOutputs = m_pOutputWidget->getNumOutputs();
	for( int i = 0; i < nOutputs; ++i )
	{
		m_qvObservedPorts.append( NULL );
		m_qvObservedPortsArrayIndex.append( 0 );
	}
}

void AppData::setInputIsReady( bool b )
{
	m_bIsInputReady = b;
}

int AppData::getNumObservedPorts()
{
	return m_pOutputWidget->getNumOutputs();
}

void AppData::updateAndDraw()
{
	updateState();
	draw();
}

void AppData::setObservedPort( OutputKernelPort* pPort, int outputIndex, int arrayIndex )
{
	m_qvObservedPorts[ outputIndex ] = pPort;
	m_qvObservedPortsArrayIndex[ outputIndex ] = arrayIndex;
}

// virtual
AppData::~AppData()
{

}

//////////////////////////////////////////////////////////////////////////
// Public Slots
//////////////////////////////////////////////////////////////////////////

void AppData::handleSaveFrameClicked()
{
	int nx;
	int ny;
	m_pOutputWidget->getNumViewports( &nx, &ny );
	
	for( int x = 0; x < nx; ++x )
	{
		for( int y = 0; y < ny; ++y )
		{
			GLTextureRectangle* pOutputTexture = m_pOutputWidget->getOutputAtViewport( x, y );
			if( pOutputTexture != NULL )
			{
				// find the next filename
				int j = 0;
				QString filename = QString( "d:/output_%1_%2__%3.png" ).arg( x ).arg( y ).arg( j );
				while( QFile::exists( filename ) )
				{
					++j;
					filename = QString( "d:/output_%1_%2__%3.png" ).arg( x ).arg( y ).arg( j );
				}

				pOutputTexture->dumpToPNG( qPrintable( filename ) );

				/*
				QString pfmFilename = QString( "d:/output%1_%2.pfm" ).arg( i ).arg( j );
				pOutputTexture->dumpToPFM( pfmFilename.toAscii().constData() );
				*/
			}
		}
	}
}

void AppData::handleDesiredFPSChanged( int period )
{
	/*
	float fps = 1000.f / period;

	Controls::getInstance()->getLabelByName( "desiredFPS" )->setText
		(
			QString( "Desired FPS: %1, Period: %2 ms" ).arg( fps ).arg( period )
		);

	setFramePeriod( period );
	*/
}

//////////////////////////////////////////////////////////////////////////
// protected
//////////////////////////////////////////////////////////////////////////

// virtual
void AppData::updateState()
{
	static int frameCount = 0;

	if( isInputReady() )
	{
		for( int i = 0; i < m_qvAutoDirtyKernelsAndPorts.size(); ++i )
		{
			QPair< QString, QString > kernelAndPort = m_qvAutoDirtyKernelsAndPorts.at( i );
			QString kernelName = kernelAndPort.first;
			QString portName = kernelAndPort.second;
			InputKernelPort* pPort =
				m_pGraph->getInputPortByKernelAndName( kernelName, portName );
			pPort->makeDirty();
		}

		m_pFBO->bind();
		glDrawBuffer( GL_COLOR_ATTACHMENT0_EXT );
		glReadBuffer( GL_COLOR_ATTACHMENT0_EXT );

		// read all observed ports
		for( int i = 0; i < getNumObservedPorts(); ++i )
		{
			if( m_qvObservedPorts[i] != NULL )
			{
				KernelPortData data = m_qvObservedPorts[i]->pullData();
				if( data.getType() == KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE )
				{
					GLTextureRectangle* pTexture = data.getGLTextureRectangleData();

#if 0
					printf( "frameCount = %d\n", frameCount );

					// stylize output
					QString filename = QString( "d:/__quadrilateral_data/e/stylize2/input_bw/input_bw_%1.png" ).arg( frameCount, 4, 10, QChar( '0' ) );
					pTexture->dumpToPNG( qPrintable( filename ) );

					if( frameCount > 715 )
					{
						exit( 0 );
					}
#endif

#if 0
					// BF output
					if( frameCount > 641 )
					{
						QString filename = QString( "d:/__quadrilateral_data/e/subsampling/bf_smoothed_%1.png" ).arg( frameCount, 4, 10, QChar( '0' ) );
						pTexture->dumpToPNG( qPrintable( filename ) );
					}

					if( frameCount > 1091 )
					{
						exit( 0 );
					}
#endif

#if 0
					// stylize input
					// QString filename = QString( "d:/data/stylize/input_bw_%1.png" ).arg( frameCount, 4, 10, QChar( '0' ) );

					// stylize output
					// QString filename = QString( "d:/data/stylize/result_bw_%1.png" ).arg( frameCount, 4, 10, QChar( '0' ) );

					// RTVA output
					// QString filename = QString( "d:/data/rtva/rtva_%1.png" ).arg( frameCount, 6, 10, QChar( '0' ) );

					// LHE output
// 					QString filename = QString( "e:/data/lhe/xray_2_lhe.png" );
// 					printf( "filename = %s\n", qPrintable( filename ) );
// 					pTexture->dumpToPNG( qPrintable( filename ) );
// 					exit( -1 );

					if( frameCount > 1445 )
					{
						exit( -1 );
					}
#else					
					m_pOutputWidget->setOutput( i, pTexture );
				}
			}
			else
			{
				m_pOutputWidget->setOutput( i, NULL );
			}
		}
		++frameCount;
	}
#endif
}

// virtual
void AppData::draw()
{
	if( isInputReady() )
	{
		GLFramebufferObject::disableAll();
		glDrawBuffer( GL_BACK );

		m_pOutputWidget->repaint();
	}
}

//////////////////////////////////////////////////////////////////////////
// private
//////////////////////////////////////////////////////////////////////////

AppData::AppData() :

	m_pOutputWidget( NULL ),
	
	m_bIsInputReady( false ),
	m_qsCgPathPrefix( "../shaders/" ),
	m_nBits( 16 ),
	
	QGameLoop( Arithmetic::roundToInt( 1000 / 25.f ) )
{

}

bool AppData::isInputReady()
{
	return m_bIsInputReady;
}

// static
AppData* AppData::s_pSingleton = NULL;
