#ifndef OUTPUTWIDGET_H
#define OUTPUTWIDGET_H

#include <GL/glew.h>

#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include <QGLWidget>
#include <QSize>
#include <time/StopWatch.h>

class CgProgramWrapper;
class GLTextureRectangle;
class QMouseEvent;

class OutputWidget : public QGLWidget
{
	Q_OBJECT

public:

	OutputWidget( int viewportWidth, int viewportHeight,
		int nViewportsX, int nViewportsY,
		QWidget* parent = 0 );
	virtual ~OutputWidget();

	// TODO: remove
	int getNumOutputs();
	void setOutput( int i, GLTextureRectangle* pOutput );

	void getNumViewports( int* pX, int* pY );
	GLTextureRectangle* getOutputAtViewport( int x, int y );
	void setOutputAtViewport( int x, int y, GLTextureRectangle* pTexture );

signals:

	void mousePressed( int x, int y, int button );
	void mouseReleased( int x, int y, int button );
	void mouseMoved( int x, int y );

protected:

	virtual void initializeGL();
	virtual void resizeGL( int w, int h );
	virtual void paintGL();
	virtual void mousePressEvent( QMouseEvent* event );
	virtual void mouseReleaseEvent( QMouseEvent* event );
	virtual void mouseMoveEvent( QMouseEvent* event );
	virtual void mouseDoubleClickEvent( QMouseEvent* event );

private:
	
	int m_iViewportWidth;
	int m_iViewportHeight;
	int m_nViewportsX;
	int m_nViewportsY;
	int m_nViewports;

	int m_iWindowWidth;
	int m_iWindowHeight;

	QVector< GLTextureRectangle* > m_qvOutputs;
	
	QVector< bool > m_qvDrawOneToOne;
	QVector< QPoint > m_qvOneToOneBottomLeft;

	// handle mouse events
	QPoint m_qpMouseDownPosition;
	int m_iMouseDownViewportIndexX;
	int m_iMouseDownViewportIndexY;
	bool m_bIsDragging;

	// Cg
	CgProgramWrapper* m_pVertexProgram;
	CGparameter m_cgp_PTV_float44_mvp;

	CgProgramWrapper* m_pFragmentProgram;
	CGparameter m_cgp_PTF_inputSampler;

	void drawOutputs();
	void drawSeparators();

	void drawOutput( int vx, int vy, GLTextureRectangle* pOutput );

	// returns the WINDOW COORDINATE of the viewport's bottom left
	void getViewportBottomLeft( int vx, int vy, int* pXOut, int* pYOut );
	
	bool isViewportOneToOne( int vx, int vy );
	void setViewportOneToOne( int vx, int vy, bool isOneToOne );

	QPoint getOneToOneImageBottomLeftForViewport( int vx, int vy );

	// given a point in WINDOW COORDINATES
	// returns the point on the image
	// returns (-1, -1) if invalid (e.g. there is no image there)
	QPoint getImagePosition( QPoint windowPosition, GLTextureRectangle** ppImage = NULL );

	// converts a position in WINDOW COORDINATES --> VIEWPORT COORDINATES
	// returns (-1, -1) if invalid
	QPoint getViewportPosition( QPoint windowPosition, int* pXIndexOut, int* pYIndexOut );

	int viewportSubscriptToIndex( int vx, int vy );

	// frame timer
	StopWatch m_stopWatch;
	int m_iFrameCount;
};

#endif // OUTPUTWIDGET_H
