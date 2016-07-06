#include "OutputWidget.h"

#include <cassert>
#include <Cg/CgProgramManager.h>
#include <Cg/CgProgramWrapper.h>
#include <Cg/CgShared.h>
#include <Cg/CgUtilities.h>
#include <GL/GLTextureRectangle.h>
#include <GL/GLUtilities.h>
#include <math/Arithmetic.h>
#include <QMouseEvent>

#include <GL/GLTexture2D.h>
#include <GL/GLFramebufferObject.h>

#include "AppData.h"


OutputWidget::OutputWidget( int viewportWidth, int viewportHeight,
						   int nViewportsX, int nViewportsY,
						   QWidget* parent ) :

	QGLWidget( parent ),

	m_iViewportWidth( viewportWidth ),
	m_iViewportHeight( viewportHeight ),

	m_nViewportsX( nViewportsX ),
	m_nViewportsY( nViewportsY ),

	m_nViewports( nViewportsX * nViewportsY ),

	m_qvOutputs( nViewportsX * nViewportsY ),
	m_qvDrawOneToOne( nViewportsX * nViewportsY ),
	m_qvOneToOneBottomLeft( nViewportsX * nViewportsY ),

	m_bIsDragging( false ),

	m_iFrameCount( 0 )
{
	// 1 pixel border between each viewport, nViewPorts - 1 borders
	int nBordersX = m_nViewportsX - 1;
	int nBordersY = m_nViewportsY - 1;

	m_iWindowWidth = nViewportsX * viewportWidth + nBordersX;
	m_iWindowHeight = nViewportsY * viewportHeight + nBordersY;

	m_qvOutputs.fill( NULL );
	m_qvDrawOneToOne.fill( false );
	m_qvOneToOneBottomLeft.fill( QPoint( 0, 0 ) );

	setFixedSize( m_iWindowWidth, m_iWindowHeight );
}

// virtual
OutputWidget::~OutputWidget()
{

}

int OutputWidget::getNumOutputs()
{
	return( m_nViewportsX * m_nViewportsY );
}

void OutputWidget::setOutput( int i, GLTextureRectangle* pOutput )
{
	m_qvOutputs[ i ] = pOutput;
}

void OutputWidget::getNumViewports( int* pX, int* pY )
{
	*pX = m_nViewportsX;
	*pY = m_nViewportsY;
}

GLTextureRectangle* OutputWidget::getOutputAtViewport( int x, int y )
{
	return m_qvOutputs.at( viewportSubscriptToIndex( x, y ) );
}

void OutputWidget::setOutputAtViewport( int x, int y, GLTextureRectangle* pTexture )
{
	m_qvOutputs[ viewportSubscriptToIndex( x, y ) ] = pTexture;
}

//////////////////////////////////////////////////////////////////////////
// Protected
//////////////////////////////////////////////////////////////////////////

// virtual
void OutputWidget::initializeGL()
{
	// global init, done in a stupid wraparound fashion
	// this widget is what creates the initial gl context
	AppData::getInstance()->initializeGL();

	QString cgPrefix = AppData::getInstance()->getCgPathPrefix();
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	CGprofile latestCgVertexProfile = CgShared::getInstance()->getLatestVertexProfile();
	CGprofile latestCgFragmentProfile = CgShared::getInstance()->getLatestFragmentProfile();

	//////////////////////////////////////////////////////////////////////////

	// ---- Vertex ----
	pProgramManager->loadProgramFromFile( "OW_passthroughVertex", cgPrefix + "Passthrough.cg",
		"passthroughVertex", latestCgVertexProfile, NULL );
	m_pVertexProgram = pProgramManager->getNamedProgram( "OW_passthroughVertex" );
	m_cgp_PTV_float44_mvp = m_pVertexProgram->getNamedParameter( "mvp" );

	// ---- Fragment ----
	pProgramManager->loadProgramFromFile( "OW_passthroughFragment", cgPrefix + "Passthrough.cg",
		"passthroughFragment", latestCgFragmentProfile, NULL );
	m_pFragmentProgram = pProgramManager->getNamedProgram( "OW_passthroughFragment" );
	m_cgp_PTF_inputSampler = m_pFragmentProgram->getNamedParameter( "inputSampler" );
}

// virtual
void OutputWidget::resizeGL( int w, int h )
{
	glViewport( 0, 0, w, h );
}

// virtual
void OutputWidget::paintGL()
{
	glClear( GL_COLOR_BUFFER_BIT );
	drawOutputs();	
	drawSeparators();

	++m_iFrameCount;
	const int nFramesPerPrint = 1;
	if( m_iFrameCount >= nFramesPerPrint )
	{
		printf( "frameTime = %f ms\n", m_stopWatch.millisecondsElapsed() / nFramesPerPrint  );
		m_stopWatch.reset();
		m_iFrameCount = 0;
	}
}

// virtual
void OutputWidget::mousePressEvent( QMouseEvent* event )
{
	Qt::KeyboardModifiers modifiers = event->modifiers();
	Qt::MouseButton button = event->button();
	QPoint position = event->pos();
	position.setY( m_iWindowHeight - position.y() - 1 );

	// on a shift click - drag the image
	// on a non-shift click, pass the message onto controls
	if( modifiers & Qt::ShiftModifier )
	{
		int vx;
		int vy;
		QPoint viewportPosition = getViewportPosition( position, &vx, &vy );
		if( viewportPosition.x() != -1 && viewportPosition.y() != -1 )
		{
			m_iMouseDownViewportIndexX = vx;
			m_iMouseDownViewportIndexY = vy;

			m_qpMouseDownPosition = position;
			m_bIsDragging = true;
		}
	}
	else
	{
		GLTextureRectangle* pImage;
		QPoint imagePosition = getImagePosition( position, &pImage );
		if( imagePosition.x() != -1 && imagePosition.y() != -1 )
		{
			// emit upside down because the images are all upside down
			emit mousePressed( imagePosition.x(), pImage->getHeight() - imagePosition.y() - 1, button );
		}
	}
}

// virtual
void OutputWidget::mouseReleaseEvent( QMouseEvent* event )
{
	Qt::KeyboardModifiers modifiers = event->modifiers();
	Qt::MouseButton button = event->button();
	// flip position
	QPoint position = event->pos();
	position.setY( m_iWindowHeight - position.y() - 1 );

	if( modifiers & Qt::ShiftModifier )
	{
		m_bIsDragging = false;
	}
	else
	{
		GLTextureRectangle* pImage;
		QPoint imagePosition = getImagePosition( position, &pImage );
		if( imagePosition.x() != -1 && imagePosition.y() != -1 )
		{
			// emit upside down because the images are all upside down
			emit mouseReleased( imagePosition.x(), pImage->getHeight() - imagePosition.y() - 1, button );
		}
	}
}

// virtual
void OutputWidget::mouseDoubleClickEvent( QMouseEvent* event )
{
	Qt::KeyboardModifiers modifiers = event->modifiers();
	Qt::MouseButton button = event->button();
	// flip position
	QPoint position = event->pos();
	position.setY( m_iWindowHeight - position.y() - 1 );

	if( modifiers & Qt::ShiftModifier )
	{
		int vx;
		int vy;
		QPoint viewportPosition = getViewportPosition( position, &vx, &vy );
		if( viewportPosition.x() != -1 && viewportPosition.y() != -1 )
		{
			bool isOneToOne = isViewportOneToOne( vx, vy );
			isOneToOne = !isOneToOne;

			// if the new state is 1-1, then center it, then constrain
			if( isOneToOne )
			{
				GLTextureRectangle* pImage = getOutputAtViewport( vx, vy );
				int imageWidth = pImage->getWidth();
				int imageHeight = pImage->getHeight();

				QPoint imagePosition = getImagePosition( position );
				QPoint bottomLeft = imagePosition - QPoint( m_iViewportWidth / 2, m_iViewportHeight / 2 );

				// constrain to within image
				// TODO: make a function
				if( bottomLeft.x() < 0 )
				{
					bottomLeft.setX( 0 );
				}
				if( bottomLeft.y() < 0 )
				{
					bottomLeft.setY( 0 );
				}

				if( ( bottomLeft.x() + m_iViewportWidth ) > imageWidth )
				{
					bottomLeft.setX( imageWidth - m_iViewportWidth );
				}

				if( ( bottomLeft.y() + m_iViewportHeight ) > imageHeight )
				{
					bottomLeft.setY( imageHeight - m_iViewportHeight );
				}
				m_qvOneToOneBottomLeft[ viewportSubscriptToIndex( vx, vy ) ] = bottomLeft;
			}

			// have to do this at the end: previous value needed to compute clicked image position
			setViewportOneToOne( vx, vy, isOneToOne );
		}
	}
}

// virtual
void OutputWidget::mouseMoveEvent( QMouseEvent* event )
{
	// flip position
	QPoint position = event->pos();
	position.setY( m_iWindowHeight - position.y() - 1 );
	
	if( m_bIsDragging )
	{		
		int mouseDownViewportIndex = viewportSubscriptToIndex
			( m_iMouseDownViewportIndexX, m_iMouseDownViewportIndexY );
		
		QPoint bottomLeft = m_qvOneToOneBottomLeft.at( mouseDownViewportIndex );
		GLTextureRectangle* pImage = m_qvOutputs.at( mouseDownViewportIndex );
		int imageWidth = pImage->getWidth();
		int imageHeight = pImage->getHeight();

		QPoint delta = m_qpMouseDownPosition - position;
		bottomLeft += delta;
		
		// constrain to within image
		if( bottomLeft.x() < 0 )
		{
			bottomLeft.setX( 0 );
		}
		if( bottomLeft.y() < 0 )
		{
			bottomLeft.setY( 0 );
		}

		if( ( bottomLeft.x() + m_iViewportWidth ) > imageWidth )
		{
			bottomLeft.setX( imageWidth - m_iViewportWidth );
		}

		if( ( bottomLeft.y() + m_iViewportHeight ) > imageHeight )
		{
			bottomLeft.setY( imageHeight - m_iViewportHeight );
		}

		m_qvOneToOneBottomLeft[ mouseDownViewportIndex ] = bottomLeft;
		
		m_qpMouseDownPosition = position;
	}
	else
	{
		GLTextureRectangle* pImage;
		QPoint imagePosition = getImagePosition( position, &pImage );
		if( imagePosition.x() != -1 && imagePosition.y() != -1 )
		{
			emit mouseMoved( imagePosition.x(), pImage->getHeight() - imagePosition.y() - 1 );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

void OutputWidget::drawOutputs()
{
	// TODO: CgUtilities::setVertexProfileEnabled( true/false, profile )
	// overload uses shared one
	cgGLEnableProfile( CgShared::getInstance()->getLatestVertexProfile() );

	for( int vy = 0; vy < m_nViewportsY; ++vy )
	{
		for( int vx = 0; vx < m_nViewportsX; ++vx )
		{
			GLTextureRectangle* pOutput = getOutputAtViewport( vx, vy );
			if( pOutput != NULL )
			{
				drawOutput( vx, vy, pOutput );
			}
		}
	}

	cgGLDisableProfile( CgShared::getInstance()->getLatestVertexProfile() );
}

void OutputWidget::drawSeparators()
{
	// TODO: this requires this thing to be aware of the state of shaders
	// move it to caller or something
	cgGLDisableProfile( CgShared::getInstance()->getLatestFragmentProfile() );

	GLUtilities::setupOrthoCamera( m_iWindowWidth, m_iWindowHeight );
	glColor3f( 1, 1, 1 );

	// draw vertical separators
	for( int vx = 0; vx < m_nViewportsX - 1; ++vx )
	{
		int x = ( vx + 1 ) * m_iViewportWidth + 1;

		glBegin( GL_LINES );

		glVertex2f( x, 0 );
		glVertex2f( x, m_iWindowHeight - 1 );

		glEnd();
	}

	// draw horizontal separators
	for( int vy = 0; vy < m_nViewportsY - 1; ++vy )
	{
		int y = ( vy + 1 ) * m_iViewportHeight + 1;

		glBegin( GL_LINES );

		glVertex2f( 0, y );
		glVertex2f( m_iWindowWidth - 1, y );

		glEnd();
	}

	cgGLEnableProfile( CgShared::getInstance()->getLatestFragmentProfile() );
}

void OutputWidget::drawOutput( int vx, int vy, GLTextureRectangle* pOutput )
{
	// setup inputs
	pOutput->setFilterMode( GLTexture::FILTER_MODE_LINEAR, GLTexture::FILTER_MODE_LINEAR );
	cgGLSetTextureParameter( m_cgp_PTF_inputSampler, pOutput->getTextureId() );

	// setup uniforms
	CgUtilities::setupOrthoCamera( m_iWindowWidth, m_iWindowHeight,
		m_cgp_PTV_float44_mvp );

	m_pVertexProgram->bind();
	m_pFragmentProgram->bind();

	// draw
	int x;
	int y;
	getViewportBottomLeft( vx, vy, &x, &y );

	int imageWidth = pOutput->getWidth();
	int imageHeight = pOutput->getHeight();

	bool isOneToOne = isViewportOneToOne( vx, vy );
	if( isOneToOne ) // drawOneToOne
	{
		QPoint imageBottomLeft = getOneToOneImageBottomLeftForViewport( vx, vy );

		glBegin( GL_QUADS );

		glTexCoord2f( imageBottomLeft.x(), imageHeight - imageBottomLeft.y() - 1 );
		glVertex2f( x, y );

		glTexCoord2f( imageBottomLeft.x() + m_iViewportWidth, imageHeight - imageBottomLeft.y() - 1 );
		glVertex2f( x + m_iViewportWidth, y );

		glTexCoord2f( imageBottomLeft.x() + m_iViewportWidth, imageHeight - imageBottomLeft.y() - 1 - m_iViewportHeight );
		glVertex2f( x + m_iViewportWidth, y + m_iViewportHeight );

		glTexCoord2f( imageBottomLeft.x(), imageHeight - imageBottomLeft.y() - 1 - m_iViewportHeight );
		glVertex2f( x, y + m_iViewportHeight );

		glEnd();
	}
	else // drawZoomed
	{
		glBegin( GL_QUADS );

		glTexCoord2f( 0, imageHeight );
		glVertex2f( x, y );

		glTexCoord2f( imageWidth, imageHeight );
		glVertex2f( x + m_iViewportWidth, y );

		glTexCoord2f( imageWidth, 0 );
		glVertex2f( x + m_iViewportWidth, y + m_iViewportHeight );

		glTexCoord2f( 0, 0 );
		glVertex2f( x, y + m_iViewportHeight );

		glEnd();
	}

	pOutput->setFilterMode( GLTexture::FILTER_MODE_NEAREST, GLTexture::FILTER_MODE_NEAREST );
}

void OutputWidget::getViewportBottomLeft( int vx, int vy, int* pXOut, int* pYOut )
{
	*pXOut = vx * ( m_iViewportWidth + 1 );
	*pYOut = vy * ( m_iViewportHeight + 1 );
}

bool OutputWidget::isViewportOneToOne( int vx, int vy )
{
	return m_qvDrawOneToOne.at( viewportSubscriptToIndex( vx, vy ) );
}

void OutputWidget::setViewportOneToOne( int vx, int vy, bool isOneToOne )
{
	m_qvDrawOneToOne[ viewportSubscriptToIndex( vx, vy ) ] = isOneToOne;
}

QPoint OutputWidget::getOneToOneImageBottomLeftForViewport( int vx, int vy )
{
	return m_qvOneToOneBottomLeft.at( viewportSubscriptToIndex( vx, vy ) );
}

QPoint OutputWidget::getImagePosition( QPoint windowPosition, GLTextureRectangle** ppImage )
{
	int vx;
	int vy;
	QPoint viewportPosition = getViewportPosition( windowPosition, &vx, &vy );

	// if invalid (-1, -1), return it
	if( viewportPosition.x() == -1 && viewportPosition.y() == -1 )
	{
		return viewportPosition;
	}
	else
	{
		GLTextureRectangle* pImage = getOutputAtViewport( vx, vy );
		if( pImage == NULL )
		{
			return QPoint( -1, -1 );
		}
		else
		{
			if( ppImage != NULL )
			{
				*ppImage = pImage;
			}

			if( isViewportOneToOne( vx, vy ) )
			{
				QPoint imageBottomLeft = getOneToOneImageBottomLeftForViewport( vx, vy );
				return imageBottomLeft + viewportPosition;
			}
			else
			{
				int imageWidth = pImage->getWidth();
				int imageHeight = pImage->getHeight();

				float fractionX = Arithmetic::divideIntsToFloat( viewportPosition.x(), m_iViewportWidth );
				float fractionY = Arithmetic::divideIntsToFloat( viewportPosition.y(), m_iViewportHeight );

				int imagePositionX = Arithmetic::roundToInt( fractionX * imageWidth );
				int imagePositionY = Arithmetic::roundToInt( fractionY * imageHeight );
				
				return QPoint( imagePositionX, imagePositionY );
			}
		}
	}
}

QPoint OutputWidget::getViewportPosition( QPoint windowPosition, int* pXIndexOut, int* pYIndexOut )
{
	// include border
	int viewportPositionX = windowPosition.x() % ( m_iViewportWidth + 1 );
	int viewportPositionY = windowPosition.y() % ( m_iViewportHeight + 1 );

	// return -1 if clicking exactly on the border
	// e.g. width = 400
	// click on border: x = 400
	// 400 % 401 = 400
	// 400 >= width is true
	if( ( viewportPositionX >= m_iViewportWidth ) ||
		( viewportPositionY >= m_iViewportHeight ) )
	{
		return QPoint( -1, -1 );
	}
	else
	{
		// guaranteed to be valid now
		*pXIndexOut = windowPosition.x() / ( m_iViewportWidth + 1 );
		*pYIndexOut = windowPosition.y() / ( m_iViewportHeight + 1 );

		return QPoint( viewportPositionX, viewportPositionY );
	}
}

int OutputWidget::viewportSubscriptToIndex( int vx, int vy )
{
	return( vy * m_nViewportsX + vx );
}
